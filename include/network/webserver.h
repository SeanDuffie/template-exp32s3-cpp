#pragma once

#include <LittleFS.h>
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "core/StorageManager.h"

extern AsyncWebServer httpServer;

void setup_webserver();

void setup_endpoints();
