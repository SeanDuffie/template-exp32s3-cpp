#include "sensors/analog_soil_moisture.h"
#include "core/debug.h"

AnalogMoistureSensor::AnalogMoistureSensor(uint8_t analogPin, uint16_t airValue, uint16_t waterValue) 
    : _pin(analogPin), _airValue(airValue), _waterValue(waterValue) {}

void AnalogMoistureSensor::begin() {
    pinMode(_pin, INPUT);
    debug_printf("Analog Moisture Sensor initialized on pin %d.\n", _pin);
}

MoistureData AnalogMoistureSensor::readData() {
    MoistureData data;
    
    // The ESP32-S3 ADC reads from 0 to 4095
    data.rawValue = analogRead(_pin);
    
    // Hardware failure check: If the signal wire breaks or shorts to ground, it usually rails to 0.
    if (data.rawValue == 0) {
        data.valid = false;
        data.percentage = 0.0;
        debug_printf("Moisture sensor read error on pin %d: Signal grounded.\n", _pin);
        return data;
    }

    data.valid = true;

    // Calculate percentage. We do this manually using floats instead of the standard map() 
    // function to prevent integer truncation errors.
    float pct = ((float)(_airValue - data.rawValue) / (float)(_airValue - _waterValue)) * 100.0;

    // Clamp the output to 0-100% in case environmental conditions exceed your calibration
    if (pct < 0.0) data.percentage = 0.0;
    else if (pct > 100.0) data.percentage = 100.0;
    else data.percentage = pct;

    return data;
}