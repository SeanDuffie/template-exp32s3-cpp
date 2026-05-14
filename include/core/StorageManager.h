#pragma once
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <SD.h>

enum StorageState {
    STORAGE_SD_ACTIVE,
    STORAGE_FALLBACK_LITTLEFS,
    STORAGE_DUMPING_TO_SD
};

class StorageManager {
private:
    fs::FS* _activeFS;
    StorageState _state;
    String _fallbackFilename;
    uint8_t _csPin;

public:
    StorageManager();
    bool begin(uint8_t csPin, uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin);
    bool appendLog(const char* filepath, const char* data);
    bool deleteFile(const char* filepath, String driveName = "littlefs");
    fs::FS* getDrive(String driveName);
    size_t getTotalBytes(String driveName);
    size_t getUsedBytes(String driveName);
    void checkSDConnection();
    void processFallbackDump();
};

extern StorageManager FileSystem;