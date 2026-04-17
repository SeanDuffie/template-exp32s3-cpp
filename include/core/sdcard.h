#pragma once
#include <SPI.h>
#include <SD.h>
#include <FS.h>

class SDManager {
private:
    SPIClass& _spi;
    uint8_t _csPin;
    bool _isMounted;

public:
    SDManager(SPIClass& spiBus, uint8_t csPin);
    
    bool begin();
    bool appendLog(const char* path, const char* message);
    uint64_t getFreeSpace();
};