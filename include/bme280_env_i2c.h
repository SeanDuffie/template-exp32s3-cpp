#pragma once
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// A clean struct to pass the data back to your main loop
struct EnvironmentData {
    float temperatureF; // °F
    float humidity;     // %
    float pressure;     // hPa
    bool valid;         // Flag to check if the sensor disconnected
};

class BME280Sensor {
private:
    Adafruit_BME280 _bme;
    TwoWire& _wire;
    uint8_t _address;

public:
    // Constructor uses Dependency Injection for the I2C bus
    BME280Sensor(TwoWire& wireBus, uint8_t address = 0x76);
    
    bool begin();
    EnvironmentData readData();
};