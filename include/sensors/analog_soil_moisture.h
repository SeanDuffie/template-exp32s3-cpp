#pragma once
#include <Arduino.h>

struct MoistureData {
    float percentage;  // 0.0% to 100.0%
    uint16_t rawValue; // Exposed for calibration purposes
    bool valid;
};

class AnalogMoistureSensor {
private:
    uint8_t _pin;
    uint16_t _airValue;
    uint16_t _waterValue;

public:
    // airValue: Raw ADC reading when completely dry in the air
    // waterValue: Raw ADC reading when submerged in a glass of water to the fill line
    AnalogMoistureSensor(uint8_t analogPin, uint16_t airValue = 3500, uint16_t waterValue = 1500);
    
    void begin();
    MoistureData readData();
};