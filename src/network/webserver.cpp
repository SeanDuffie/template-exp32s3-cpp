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

    // httpServer.on("/api/networks", HTTP_GET, [](AsyncWebServerRequest *request){
    //     update_requested = true;
    //     request->send(200, "application/json", "{\"status\":\"updating\"}");
    // });
    // httpServer.on("/api/connect", HTTP_POST, [](AsyncWebServerRequest *request){
    //     update_requested = true;
    //     request->send(200, "application/json", "{\"status\":\"updating\"}");
    // });

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
    httpServer.on("/api/files", HTTP_GET, [](AsyncWebServerRequest *request) {
        String drive = request->hasParam("drive") ? request->getParam("drive")->value() : "littlefs";

        JsonDocument doc;

        // Ask the StorageManager for the filesystem
        fs::FS* targetFS = FileSystem.getDrive(drive);

        // Catch the null pointer if the drive is disconnected
        if (targetFS == nullptr) {
            request->send(503, "application/json", "{\"error\":\"Hardware disconnected\"}");
            return;
        }

        doc["totalBytes"] = FileSystem.getTotalBytes(drive);
        doc["usedBytes"] = FileSystem.getUsedBytes(drive);

        JsonObject files = doc["files"].to<JsonObject>();

        File root = targetFS->open("/");
        if (!root || !root.isDirectory()) {
            request->send(500, "application/json", "{\"error\":\"Failed to open directory\"}");
            return;
        }

        File file = root.openNextFile();
        while (file) {
            String fileName = String(file.name());
            if (fileName.startsWith("/")) fileName = fileName.substring(1);

            JsonObject fileObj = files[fileName].to<JsonObject>();
            fileObj["size"] = file.size();
            file = root.openNextFile();
        }

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // 2. File Download Endpoint
    httpServer.on("/api/download", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!request->hasParam("file")) {
            request->send(400, "text/plain", "Bad Request: Missing file parameter");
            return;
        }
        String filename = "/" + request->getParam("file")->value();
        String drive = request->hasParam("drive") ? request->getParam("drive")->value() : "littlefs";

        // Ask the StorageManager for the filesystem
        fs::FS* targetFS = FileSystem.getDrive(drive);

        // Swap 'LittleFS' for 'SD' here as well
        if (targetFS->exists(filename)) {
            // The 'true' boolean forces the browser to download instead of displaying it inline
            request->send(*targetFS, filename, "application/octet-stream", true); 
        } else {
            request->send(404, "text/plain", "File Not Found");
        }
    });

    // 3. File Delete Endpoint
    httpServer.on("/api/delete", HTTP_DELETE, [](AsyncWebServerRequest *request) {
        if (!request->hasParam("file")) {
            request->send(400, "text/plain", "Bad Request: Missing file parameter");
            return;
        }
        String filename = "/" + request->getParam("file")->value();
        String drive = request->hasParam("drive") ? request->getParam("drive")->value() : "littlefs";

        // Ask the StorageManager for the filesystem
        fs::FS* targetFS = FileSystem.getDrive(drive);

        // Swap 'LittleFS' for 'SD' here as well
        if (targetFS->exists(filename)) {
            debug_printf("WebServer: Deleting file '%s' from drive '%s'\n", filename.c_str(), drive.c_str());
            FileSystem.deleteFile(filename.c_str(), drive);
        } else {
            request->send(404, "text/plain", "File Not Found");
        }
    });
}
