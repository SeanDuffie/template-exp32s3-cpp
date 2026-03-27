#include "hcsr04_dist.h"
#include "debug.h"

HCSR04Sensor::HCSR04Sensor(uint8_t trigPin, uint8_t echoPin, unsigned long timeoutUs) 
    : _trigPin(trigPin), _echoPin(echoPin), _timeoutUs(timeoutUs) {}

void HCSR04Sensor::begin() {
    pinMode(_trigPin, OUTPUT);
    pinMode(_echoPin, INPUT);
    
    // Ensure trigger is low to start
    digitalWrite(_trigPin, LOW);
    
    debug_printf("HC-SR04 initialized. Trig: %d, Echo: %d\n", _trigPin, _echoPin);
}

DistanceData HCSR04Sensor::readData() {
    DistanceData data;

    // 1. Clear the trigger pin
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);

    // 2. Send a 10 microsecond HIGH pulse to trigger the burst
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigPin, LOW);

    // 3. Measure the duration of the HIGH pulse on the Echo pin
    unsigned long duration = pulseIn(_echoPin, HIGH, _timeoutUs);

    // 4. Calculate distance
    if (duration == 0) {
        data.valid = false;
        data.distanceCM = 0.0;
        data.distanceInches = 0.0;
        debug_println("HC-SR04 read error: Timeout or out of range.");
    } else {
        data.valid = true;
        // Speed of sound is ~343 m/s -> 0.0343 cm/us. Divide by 2 for round trip.
        data.distanceCM = (duration * 0.0343) / 2.0;
        data.distanceInches = data.distanceCM * 0.393701;
    }

    return data;
}