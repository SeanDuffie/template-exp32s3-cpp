#include "sensors/neo6m_gps.h"
#include "core/debug.h"

#define U1_RX_PIN 17
#define U1_TX_PIN 18

NEO6MSensor::NEO6MSensor(HardwareSerial& serialBus) 
    : _serial(serialBus) {}

void NEO6MSensor::begin() {
    
    Serial1.begin(9600, SERIAL_8N1, U1_RX_PIN, U1_TX_PIN); // ESP32 to GPS
    // Serial.println("GPS Passthrough Started");
    // U-blox command to enable standard NMEA sentences and disable UBX binary
    // const byte NMEA_ON[] = {
    //     0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 
    //     0xD0, 0x08, 0x00, 0x00, 0x80, 0x25, 0x00, 0x00, 0x07, 0x00, 
    //     0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xA9
    // };
    // Serial1.write(NMEA_ON, sizeof(NMEA_ON));
    debug_println("NEO-6M GPS driver initialized. Awaiting satellite fix...");
}

void NEO6MSensor::update() {
    // // Drain the hardware serial buffer and feed the parser.
    while (Serial1.available() > 0) {
        _gps.encode(Serial1.read());
    }
    
    /** DEBUG: Read raw data from the Serial port. */
    // while (Serial1.available()) {
    //     Serial.write(Serial1.read());
    // }
    // while (Serial.available()) {
    //     Serial1.write(Serial.read());
    // }
}

GPSData NEO6MSensor::readData() {
    GPSData data;

    // isValid() is only true when the GPS has locked onto enough satellites
    data.valid = _gps.location.isValid();
    
    // You can sometimes see satellites before you have a valid location fix
    data.satellites = _gps.satellites.isValid() ? _gps.satellites.value() : 0;

    if (data.valid) {
        data.latitude = _gps.location.lat();
        data.longitude = _gps.location.lng();
        data.altitudeMeters = _gps.altitude.isValid() ? _gps.altitude.meters() : 0.0;
        data.speedMPH = _gps.speed.isValid() ? _gps.speed.mph() : 0.0;
        
        /** GPS Timestamp Info */
        data.year = _gps.date.year();
        data.month = _gps.date.month();
        data.day = _gps.date.day();
        data.hour = _gps.time.hour();
        data.minute = _gps.time.minute();
        data.second = _gps.time.second();
    } else {
        data.latitude = -1;
        data.longitude = -1;
        data.altitudeMeters = -1;
        data.speedMPH = -1;

        /** GPS Timestamp Info */
        data.year = -1;
        data.month = -1;
        data.day = -1;
        data.hour = -1;
        data.minute = -1;
        data.second = -1;
    }

    return data;
}