#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#define RGB_LED_PIN 48 // Default WS2812 pin for ESP32-S3 DevKitC

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

void setup_wifi() {
    Serial.print("Connecting to Wi-Fi");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected. IP Address: ");
    Serial.println(WiFi.localIP());
}

void setup_ota() {
    // Port defaults to 3232
    // ArduinoOTA.setPort(3232);

    // Hostname defaults to esp3232-[MAC]
    // ArduinoOTA.setHostname("esp32-s3-template");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }
        // NOTE: if updating SPIFFS/LittleFS this would be the place to unmount it
        Serial.println("Start updating " + type);
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
    Serial.println("OTA Ready.");
}

void setup() {
    Serial.begin(115200);
    // Wait for native USB CDC connection
    while (!Serial && millis() < 5000) { delay(10); }

    // 1. Establish Wi-Fi (Blocking)
    setup_wifi();
    Serial.printf("\nConnected. IP: %s\n", WiFi.localIP().toString().c_str());

    // 2. Initialize OTA Server
    setup_ota();
}

void loop() {
    // Process incoming OTA requests
    ArduinoOTA.handle();

    // Blink LED and send heartbeat every 1000ms without blocking OTA
    static uint32_t last_millis = 0;
    if (millis() - last_millis >= 1000) {
        last_millis = millis();
        
        static bool led_state = false;
        led_state = !led_state;
        
        if (led_state) {
            neopixelWrite(RGB_LED_PIN, 0, 50, 0); // Green
        } else {
            neopixelWrite(RGB_LED_PIN, 0, 0, 0);  // Off
        }

        Serial.println("Heartbeat: System Nominal");
    }
}