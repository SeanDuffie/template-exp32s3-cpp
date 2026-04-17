#pragma once
#include <Arduino.h>

struct DistanceData {
    float distanceCM;
    float distanceInches;
    bool valid;
};

class HCSR04Sensor {
private:
    uint8_t _trigPin;
    uint8_t _echoPin;
    unsigned long _timeoutUs;

public:
    // timeoutUs: 30000 microseconds is roughly a 5-meter max range timeout.
    HCSR04Sensor(uint8_t trigPin, uint8_t echoPin, unsigned long timeoutUs = 30000);
    
    void begin();
    DistanceData readData();
};