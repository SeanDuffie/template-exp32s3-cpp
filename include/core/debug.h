#pragma once
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

#include "core/StorageManager.h"

// Initialize Servers
extern AsyncWebSocket ws;
extern WiFiServer telnetServer;
extern bool update_requested;
extern String log_buffer;

void setup_debug();
void debug_print(const char* message);
void debug_println(const char* message);
void debug_printf(const char* format, ...);
void handle_telnet();
