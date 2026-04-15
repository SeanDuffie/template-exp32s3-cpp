#pragma once
#include <HardwareSerial.h>
#include <TinyGPS++.h>

struct GPSData {
    double latitude;
    double longitude;
    float altitudeMeters;
    float speedMPH;
    uint32_t satellites;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    bool valid; // True only when the module has a hard satellite fix
};

/** NOTE: This sensor requires 5V on VCC, not 3.3V. */
class NEO6MSensor {
private:
    HardwareSerial& _serial;
    TinyGPSPlus _gps;

public:
    NEO6MSensor(HardwareSerial& serialBus);

    void begin();

    void update(); 

    GPSData readData();
};