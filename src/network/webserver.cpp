/**
 * Handles static file serving, API routes, and LittleFS.
 */
#include "network/webserver.h"
#include "core/debug.h" // Gives access to 'ws' and 'update_requested'

AsyncWebServer httpServer(80);

void setup_webserver() {
    // 1. Initialize File System
    if(!LittleFS.begin(true)){
        debug_println("An Error has occurred while mounting LittleFS");
        return;
    }

    // 2. Define API Routes
    httpServer.on("/api/update", HTTP_POST, [](AsyncWebServerRequest *request){
        update_requested = true;
        request->send(200, "application/json", "{\"status\":\"updating\"}");
    });

    // Note: You still need to implement the POST handler for /api/wifi here. 
    // This requires a JSON parsing library like AsyncJson to read the payload.

    // 3. Mount WebSockets (Defined in debug.cpp)
    httpServer.addHandler(&ws);

    // 4. Serve Frontend
    httpServer.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // 5. Start Server
    httpServer.begin();
    debug_println("Web server started.");
}

void setup_endpoints() {
    // 1. List Files Endpoint
    server.on("/api/files", HTTP_GET, [](AsyncWebServerRequest *request) {
        JsonDocument doc;
        JsonArray files = doc["files"].to<JsonArray>();

        // Swap 'LittleFS' for 'SD' when your hardware changes
        File root = LittleFS.open("/");
        if (!root || !root.isDirectory()) {
            request->send(500, "application/json", "{\"error\":\"Failed to open directory\"}");
            return;
        }

        File file = root.openNextFile();
        while (file) {
            JsonObject obj = files.add<JsonObject>();
            // LittleFS includes the leading slash in the name, strip it for cleaner UI
            String fileName = String(file.name());
            if (fileName.startsWith("/")) fileName = fileName.substring(1);
            
            obj["name"] = fileName;
            obj["size"] = file.size();
            file = root.openNextFile();
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // 2. File Download Endpoint
    server.on("/api/download", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("file")) {
            String filename = "/" + request->getParam("file")->value();
            
            // Swap 'LittleFS' for 'SD' here as well
            if (LittleFS.exists(filename)) {
                // The 'true' boolean forces the browser to download instead of displaying it inline
                request->send(LittleFS, filename, "application/octet-stream", true); 
            } else {
                request->send(404, "text/plain", "File Not Found");
            }
        } else {
            request->send(400, "text/plain", "Bad Request: Missing file parameter");
        }
    });
}
