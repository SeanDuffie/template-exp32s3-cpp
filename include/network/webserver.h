#pragma once

#include <LittleFS.h>
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
// #include <SD.h> // Uncomment when SD module arrives

extern AsyncWebServer httpServer;
extern AsyncWebServer server;

void setup_webserver();

void setup_endpoints();
