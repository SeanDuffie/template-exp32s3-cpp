#pragma once
// #include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "core/debug.h"

// A clean struct to pass the data back to your main loop
struct AirData {
    float tempC;        // °C
    float tempF;        // °F
    float humidity;     // %
    float pressure;     // hPa
    bool valid;         // Flag to check if the sensor disconnected
};

class BME280Sensor {
private:
    Adafruit_BME280 _bme;
    // uint8_t _csPin;
    TwoWire& _wire;
    uint8_t _address;

public:
    // Constructor uses Dependency Injection
    // BME280Sensor(SPIClass& spiBus, uint8_t csPin);
    BME280Sensor(TwoWire& wireBus, uint8_t address = 0x76);
    bool begin();
    AirData readData();
};