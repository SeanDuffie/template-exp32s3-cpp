#pragma once
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

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
    uint8_t _csPin;

public:
    // Constructor uses Dependency Injection for the I2C bus
    BME280Sensor(SPIClass& spiBus, uint8_t csPin);

    bool begin();
    AirData readData();
};