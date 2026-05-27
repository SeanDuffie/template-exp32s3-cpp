#pragma once
#include <Arduino.h>

class SensorStats {
private:
    float sum;
    float minimum;
    float maximum;
    uint32_t count;

public:
    SensorStats() { reset(); }

    void add(float value) {
        sum += value;
        if (count == 0 || value < minimum) minimum = value;
        if (count == 0 || value > maximum) maximum = value;
        count++;
    }

    float mean() { return count == 0 ? 0.0 : sum / count; }
    float getMin() { return count == 0 ? 0.0 : minimum; }
    float getMax() { return count == 0 ? 0.0 : maximum; }

    void reset() {
        sum = 0;
        minimum = 0;
        maximum = 0;
        count = 0;
    }
};