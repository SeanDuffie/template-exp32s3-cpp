#include "core/sdcard.h"
#include "core/debug.h"

SDManager::SDManager(SPIClass& spiBus, uint8_t csPin) 
    : _spi(spiBus), _csPin(csPin), _isMounted(false) {}

bool SDManager::begin() {
    // The default SPI frequency for SD is 4MHz. 
    // The mount point defaults to "/sd".
    if (!SD.begin(_csPin, _spi)) {
        debug_println("SD Card Mount Failed. Check wiring and formatting.");
        _isMounted = false;
        return false;
    }
    
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        debug_println("No SD Card attached.");
        _isMounted = false;
        return false;
    }
    
    debug_printf("SD Card initialized. Type: %d\n", cardType);
    _isMounted = true;
    return true;
}

bool SDManager::appendLog(const char* path, const char* message) {
    if (!_isMounted) {
        debug_println("Cannot write: SD card not mounted.");
        return false;
    }
    
    // FILE_APPEND creates the file if it does not exist
    File file = SD.open(path, FILE_APPEND);
    if (!file) {
        debug_printf("Failed to open %s for appending.\n", path);
        return false;
    }
    
    if (!file.println(message)) {
        debug_println("Append failed. Card may be full or corrupted.");
        file.close();
        return false;
    }
    
    file.close();
    return true;
}

uint64_t SDManager::getFreeSpace() {
    if (!_isMounted) return 0;
    return SD.totalBytes() - SD.usedBytes();
}