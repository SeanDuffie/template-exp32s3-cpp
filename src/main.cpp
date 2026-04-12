#include <Arduino.h>
#include <SPI.h>
#include <ArduinoJson.h>

#include "debug.h"

#include "network.h"
#include "ota.h"
#include "webserver.h"

#include "neo6m_gps.h"

/** Available Left Pins */
// #define GND 1
#define PWR 2 // For some reason the dev kit has 2 PWR instead of 1 being GND
#define EN 3
#define ONE_WIRE_BUS_PIN 4
#define ANALOG_PIN 5
#define GPIO6 6
#define GPIO7 7
#define US_TRIG_PIN 15
#define US_ECHO_PIN 16
#define GPIO8 8


/** Available Bottom Pins */
#define STRP3 3 // Floating Strapping Pin
#define STRP46 46 // Pull Down Strapping Pin
#define I2C_SCA 9
#define I2C_SCL 10
#define SPI_CS 11
#define SPI_SCK 12
#define SPI_MOSI 13
#define SPI_MISO 14
#define GPIO21 21
#define GPIO47 47
#define GPIO48 48
#define GPIO45 45 // Pull Down Strapping Pin

/** Available Right Pins */
#define STRP0 0 // Pull Up Strapping Pin
#define GPIO35 35 // Do not use with Octal PSRAM (8MB)
#define GPIO36 36 // Do not use with Octal PSRAM (8MB)
#define GPIO37 37 // Do not use with Octal PSRAM (8MB)
#define TODO 38
#define MTCK 39 // JTAG
#define MTD0 40 // JTAG
#define MTDI 41 // JTAG
#define MTMS 42 // JTAG
#define U0_RXD 44 // Native USB
#define U0_TXD 43 // Native USB
#define GPIO2 2
#define GPIO1 1

static bool led_state = false;
#define RGB_LED_PIN 38

// UART
// Instantiate Serial1 and inject it into the sensor class
HardwareSerial GPS_Serial(1);
NEO6MSensor myGPS(GPS_Serial);

void setup() {
    Serial.begin(115200);
    // Wait for native USB CDC connection
    while (!Serial && millis() < 5000) { delay(10); }

    // 1. Establish Wi-Fi (Blocking)
    setup_wifi_manager();
    // 2. Establish Debugging ()
    setup_debug();
    // 3. Initializes LittleFS and endpoints
    setup_webserver();
    // 4. Initialize OTA Server
    setup_ota();
    // 5. Pull time from NTP server
    setup_ntp();

    myGPS.begin();
}


void loop() {
    // Process incoming OTA requests
    ArduinoOTA.handle();
    
    // Purge abandoned WebSocket connections from page refreshes
    ws.cleanupClients();

    myGPS.update();     // DRAIN THE BUFFER: This must run every single loop iteration, unblocked.

    // Blink LED and send heartbeat every 1000ms without blocking OTA
    static uint32_t last_millis = 0;
    if (millis() - last_millis >= 1000) {
        // debug_println("-----------------------------------");

        // Create JSON Document (Size 1024 to be safe)
        JsonDocument doc;

        String tstmp = get_timestamp();
        debug_printf("Current Time: %s\n", tstmp.c_str());
        last_millis = millis();

        // JSONify System Metadata
        doc["timestamp"] = tstmp.c_str();
        doc["uptime"] = last_millis / 1000;

        // Add connected hardware flags
        JsonObject active = doc["active_sensors"].to<JsonObject>();
        active["ds18b20"] = false;
        active["soil"] = false;
        active["bme280"] = false;
        active["bh1750"] = false;
        active["gps"] = true;
        active["imu"] = false;

        /** RGB LED SECTION */
        led_state = !led_state;
        neopixelWrite(RGB_LED_PIN, led_state ? 1 : 0, 0, 0);

        // debug_println("Heartbeat: System Nominal");
        
        /** GPS SECTION */
        GPSData loc = myGPS.readData(); // READ THE DATA: Only pull the data out during your timed heartbeat.
        if (loc.valid) {
            debug_printf(
                "GPS UTC Time: %04d-%02d-%02d %02d:%02d:%02d\n",
                loc.year, loc.month, loc.day, loc.hour, loc.minute, loc.second
            );
            debug_printf("Lat: %f, Lon: %f, Sats: %d\n",
                loc.latitude, loc.longitude, loc.satellites
            );
        } else {
            debug_printf("Waiting for GPS fix... Satellites in view: %d\n", loc.satellites);
        }

        // GPS Data
        JsonObject gps = doc["gps"].to<JsonObject>();

        char tstmp_buffer[20] = "";
        sprintf(tstmp_buffer, "%04d-%02d-%02d %02d:%02d:%02d", loc.year, loc.month, loc.day, loc.hour, loc.minute, loc.second);
        gps["time"] = tstmp_buffer;

        gps["latitude"] = loc.latitude;
        gps["longitude"] = loc.longitude;
        gps["altitude"] = loc.altitudeMeters;
        gps["speed"] = loc.speedMPH;
        gps["satellites"] = loc.satellites;
        // gps["gps_timestamp"] = TODO;

        doc["logs"] = log_buffer;

        // Serialize and Send to Webpage
        char buffer[2048];
        serializeJson(doc, buffer);
        ws.textAll(buffer);
        log_buffer = "";

        handle_telnet();
    }

    /** OTA SECTION */
    if (update_requested) {
        update_requested = false; // Reset flag immediately
        debug_println("Manual OTA update triggered by user.");
        perform_ota_update(); // Your HTTPUpdate logic
    }
}
