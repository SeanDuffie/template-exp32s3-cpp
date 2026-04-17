/**
 * Handles the captive portal, Wi-Fi connections, and physical resets.
 */
#include <time.h>
#include <map>
#include "network/network.h"
#include "network/webserver.h"
#include "core/debug.h"

// Define a temporary DNS server for the captive portal
DNSServer dns;

const char* ntpServer = "pool.ntp.org";

void setup_wifi_manager() {
    Serial.println("Starting Wi-Fi Manager...");

    // Local intialization. Once its business is done, there is no need to keep it around
    // Pass the async web server and dns server to the manager
    AsyncWiFiManager wm(&httpServer, &dns);

    // Set a custom hostname for the ESP32 on your network
    WiFi.setHostname("ESP32-S3-Template-cpp");

    // Check if the BOOT button is held down while the device turns on
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
    delay(100); // Brief debounce
    
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
        Serial.println("Boot button held. Wiping Wi-Fi credentials...");
        wm.resetSettings();
        
        // Blink LED rapidly to indicate wipe is complete
        for(int i = 0; i < 10; i++) {
            neopixelWrite(RGB_LED_PIN, 50, 0, 0); // Red
            delay(100);
            neopixelWrite(RGB_LED_PIN, 0, 0, 0);
            delay(100);
        }
    }

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name
    // and goes into a blocking loop awaiting configuration
    if (!wm.autoConnect("ESP32-Setup-Portal")) {
        Serial.println("Failed to connect and hit timeout");
        ESP.restart(); // Reset and try again
    } 
    
    // If you get here you have connected to the WiFi
    Serial.println("\nConnected to Wi-Fi.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

long get_offset_from_code(String code) {
    // Map of 3-character codes to offsets in seconds
    static std::map<String, long> tz_map = {
        {"UTC", 0},
        {"EST", -18000}, // Eastern Standard (-5h)
        {"EDT", -14400}, // Eastern Daylight (-4h)
        {"CST", -21600}, // Central Standard (-6h)
        {"CDT", -18000}, // Central Daylight (-5h)
        {"MST", -25200}, // Mountain Standard (-7h)
        {"MDT", -21600}, // Mountain Daylight (-6h)
        {"PST", -28800}, // Pacific Standard (-8h)
        {"PDT", -25200}  // Pacific Daylight (-7h)
    };

    if (tz_map.find(code) != tz_map.end()) {
        return tz_map[code];
    }
    
    return -18000; // Default to EST if not found
}

void setup_ntp(String code, bool DST) {
    const long  gmtOffset_sec = get_offset_from_code(code); // EST is -5 hours. Adjust for your timezone.
    const int   daylightOffset_sec = 3600 * DST; // 1 hour for DST
    // configTime is built into the ESP32 core
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // Block and wait for the time to actually sync
    struct tm timeinfo;
    int retry = 0;
    
    // Check every 500ms, up to 10 times (5 seconds total)
    while (!getLocalTime(&timeinfo) && retry < 10) {
        debug_print(".");
        delay(500);
        retry++;
    }

    if (retry < 10) {
        debug_println("\nNTP synchronized successfully!");
    } else {
        debug_println("\nNTP Sync Failed. Timestamp will be N/A until background sync completes.");
    }
    Serial.println("NTP Sync Initiated...");
}

String get_timestamp() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        return "N/A";
    }
    char buffer[25];
    // Formats as YYYY-MM-DD HH:MM:SS
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buffer);
}
