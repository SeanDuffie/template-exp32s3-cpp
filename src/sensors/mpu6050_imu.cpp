#include "sensors/mpu6050_imu.h"
#include "core/debug.h"

MPU6050Sensor::MPU6050Sensor(TwoWire& wireBus, uint8_t address) 
    : _wire(wireBus), _address(address) {}

bool MPU6050Sensor::begin() {
    if (!_mpu.begin(_address, &_wire, 0)) {
        debug_printf("MPU6050 initialization failed at address 0x%02X.\n", _address);
        return false;
    }

    // // Write 0x00 to PWR_MGMT_1 (Register 0x6B) to wake the sensor
    // _wire.beginTransmission(_address);
    // _wire.write(0x6B);
    // _wire.write(0x00);
    // _wire.endTransmission();

    // Standard configuration for smooth, general-purpose movement tracking
    _mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    _mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    _mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    debug_printf("MPU6050 initialized successfully at 0x%02X.\n", _address);
    return true;
}

IMUData MPU6050Sensor::readData() {
    IMUData data;
    sensors_event_t a, g, temp;
    
    // Fetch the raw data from the Adafruit driver
    if (_mpu.getEvent(&a, &g, &temp)) {
        data.valid = true;
        
        data.accelX = a.acceleration.x;
        data.accelY = a.acceleration.y;
        data.accelZ = a.acceleration.z;
        
        data.gyroX = g.gyro.x;
        data.gyroY = g.gyro.y;
        data.gyroZ = g.gyro.z;
        
        // The MPU6050 has a built-in temperature sensor, mostly used to calibrate the IMU drift
        data.temperatureF = (temp.temperature * 9.0 / 5.0) + 32.0;
    } else {
        data.valid = false;
        debug_println("MPU6050 read error: Failed to get event.");
    }

    return data;
}