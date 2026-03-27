#pragma once
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

struct IMUData {
    float accelX, accelY, accelZ; // m/s^2
    float gyroX, gyroY, gyroZ;    // rad/s
    float temperatureF;           // °F
    bool valid;
};

class MPU6050Sensor {
private:
    Adafruit_MPU6050 _mpu;
    TwoWire& _wire;
    uint8_t _address;

public:
    // The GY-521 defaults to 0x68. If the AD0 pin is pulled to 3.3V, it becomes 0x69.
    MPU6050Sensor(TwoWire& wireBus, uint8_t address = 0x68);
    
    bool begin();
    IMUData readData();
};