#include <core/StorageManager.h>

StorageManager FileSystem;

StorageManager::StorageManager() : _activeFS(nullptr), _state(STORAGE_FALLBACK_LITTLEFS), _fallbackFilename("/fallback_log.csv") {}

bool StorageManager::begin(uint8_t csPin, uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin) {
    this->_csPin = csPin;  // Store CS pin for later use in SD reinitialization

    // bool littleFsMounted = LittleFS.begin(true);
    bool littleFsMounted = LittleFS.begin();
    if (!littleFsMounted) {
        debug_println("StorageManager: CRITICAL ERROR - LittleFS failed to mount.");
        // The web server cannot function without LittleFS.
    }

    // Route the hardware SPI bus to your custom S3 pins
    SPI.begin(sckPin, misoPin, mosiPin, _csPin);

    // Always attempt to mount SD, regardless of LittleFS state
    if (SD.begin(_csPin, SPI, 400000)) { // Replace 5 with your actual CS Pin
        _activeFS = &SD;
        _state = STORAGE_SD_ACTIVE;
        debug_println("StorageManager: SD card active.");
    } else {
        _activeFS = &LittleFS;
        _state = STORAGE_FALLBACK_LITTLEFS;
        debug_println("StorageManager: SD failed. Falling back to LittleFS.");
    }

    return littleFsMounted;
}

bool StorageManager::appendLog(const char* filepath, const char* data) {
    if (_activeFS == nullptr) return false;

    File file = _activeFS->open(filepath, FILE_APPEND);
    if (!file) {
        // If writing to SD fails, force the fallback state
        if (_state == STORAGE_SD_ACTIVE) {
            debug_println("StorageManager: SD write failed. Forcing LittleFS fallback.");
            _activeFS = &LittleFS;
            _state = STORAGE_FALLBACK_LITTLEFS;
            // Retry the write on LittleFS
            return appendLog(_fallbackFilename.c_str(), data); 
        }
        return false;
    }

    if (!file.println(data)) {
        debug_println("StorageManager: Append failed. Storage full.");
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool StorageManager::deleteFile(const char* filepath, String driveName) {
    fs::FS* targetFS = getDrive(driveName);

    if (targetFS == nullptr) {
        debug_printf("StorageManager: Failed to delete file '%s' on drive '%s' - drive unavailable.\n", filepath, driveName.c_str());
        return false;
    }

    bool result = targetFS->remove(filepath);
    if (result) {
        if (targetFS->exists(filepath)) {
            debug_printf("StorageManager: Failed to delete file '%s' on drive '%s' - file still exists after deletion attempt.\n", filepath, driveName.c_str());
            return false;
        } else {
            debug_printf("StorageManager: Successfully deleted file '%s' from drive '%s'.\n", filepath, driveName.c_str());
        }
    } else {
        debug_printf("StorageManager: Failed to delete file '%s' from drive '%s' - remove() returned false.\n", filepath, driveName.c_str());
    }

    return result;
}

fs::FS* StorageManager::getDrive(String driveName) {
    if (driveName == "sd") {
        if (_state == STORAGE_SD_ACTIVE) return &SD;
        return nullptr; // SD requested but currently unavailable
    }
    return &LittleFS;
}

uint64_t StorageManager::getTotalBytes(String driveName) {
    if (driveName == "sd") {
        if (_state == STORAGE_SD_ACTIVE) return SD.totalBytes();
        return 0; // Return 0 if the SD card is currently disconnected
    }
    return LittleFS.totalBytes();
}

uint64_t StorageManager::getUsedBytes(String driveName) {
    if (driveName == "sd") {
        if (_state == STORAGE_SD_ACTIVE) return SD.usedBytes();
        return 0; 
    }
    return LittleFS.usedBytes();
}

void StorageManager::checkSDConnection() {
    // Only attempt to remount if we are currently in a fallback state
    if (_state == STORAGE_FALLBACK_LITTLEFS) {
        if (SD.begin(_csPin, SPI)) {
            debug_println("StorageManager: SD card reconnected.");
            _activeFS = &SD;
            _state = STORAGE_SD_ACTIVE;
            processFallbackDump();
        }
    }
}

void StorageManager::processFallbackDump() {
    if (!LittleFS.exists(_fallbackFilename)) return;

    File fallbackFile = LittleFS.open(_fallbackFilename, FILE_READ);
    if (!fallbackFile) return;

    File sdFile = SD.open(_fallbackFilename, FILE_APPEND);
    if (!sdFile) {
        debug_println("StorageManager: Failed to open SD for dumping.");
        fallbackFile.close();
        return;
    }

    debug_println("StorageManager: Dumping fallback data to SD...");

    // Chunked transfer to prevent Watchdog Timer crashes
    uint8_t buffer[512];
    size_t bytesRead;
    
    while ((bytesRead = fallbackFile.read(buffer, sizeof(buffer))) > 0) {
        sdFile.write(buffer, bytesRead);
        yield(); // Feeds the watchdog and allows background Wi-Fi tasks to run
    }

    sdFile.close();
    fallbackFile.close();
    
    LittleFS.remove(_fallbackFilename);
    debug_println("StorageManager: Dump complete. Fallback file deleted.");
}

// TODO: Get a smarter way to predict row size
uint8_t StorageManager::getBytesPerRow(String driveName) {
    // For simplicity, return fixed values. In a real implementation, this could be dynamic or configurable.
    return 80; // Default to 80 bytes per row
}

// TODO: Get a smarter way to predict interval
uint8_t StorageManager::getIntervalMinutes(String driveName) {
    // For simplicity, return fixed values. In a real implementation, this could be dynamic or configurable.
    return 15; // Default to 15 minutes
}