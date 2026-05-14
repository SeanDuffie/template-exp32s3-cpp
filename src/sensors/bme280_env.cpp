#include "sensors/bme280_env.h"

// BME280Sensor::BME280Sensor(SPIClass& spiBus, uint8_t csPin) 
//     : _bme(csPin, &spiBus), _csPin(csPin) {}
BME280Sensor::BME280Sensor(TwoWire& wireBus, uint8_t address) 
    : _wire(wireBus), _address(address) {}

bool BME280Sensor::begin() {
    // The Adafruit library accepts the I2C address and a pointer to the shared bus
    // if (!_bme.begin()) {
    //     debug_printf("BME280 (SPI) initialization failed on CS pin %d.\n", _csPin);
    //     return false;
    if (!_bme.begin(_address, &_wire)) {
        debug_println("BME280 initialization failed. Check wiring and I2C address.");
        return false;
    }

    // Recommended settings for general air monitoring
    _bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                     Adafruit_BME280::SAMPLING_X1,  // Temperature
                     Adafruit_BME280::SAMPLING_X1,  // Pressure
                     Adafruit_BME280::SAMPLING_X1,  // Humidity
                     Adafruit_BME280::FILTER_OFF);

    debug_println("BME280 initialized successfully.");
    return true;
}

AirData BME280Sensor::readData() {
    AirData data;

    data.tempC = _bme.readTemperature();
    data.tempF = (data.tempC * 9.0 / 5.0) + 32.0; 

    data.humidity = _bme.readHumidity();
    data.pressure = _bme.readPressure() / 100.0F; // Convert Pa to hPa

    // Validate the readings (BME280 returns NaN on hardware failure)
    if (isnan(data.tempC) || isnan(data.humidity) || isnan(data.pressure)) {
        data.valid = false;
        debug_println("BME280 read error: NaN returned.");
        begin(); // Attempt to reinitialize the sensor on read failure
    } else {
        data.valid = true;
    }

    return data;
}