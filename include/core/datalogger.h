#pragma once
#include "core/debug.h"
#include "core/StorageManager.h"
#include "core/sensor_stats.h"

// TODO: Should this eventually log binary files so that it skips serialization and is more space efficient/consistent?
// It would require a custom parser on the frontend though, and would make it less human readable when inspecting the files directly on the device. Maybe add an option for both text and binary logging?

// MARK: Modify for enabled sensors
struct sDataRow {
    String timestamp;
    unsigned long uptime;
    float lux;
    // float soilMoisture;
    // float soilMoistureADC;
    // float soilTempC;
    // float soilTempF;
    // float airTempC;
    // float AirTempF;
    // float humidity;
    // float pressure;
    // float accelX;
    // float accelY;
    // float accelZ;
    // float gyroX;
    // float gyroY;
    // float gyroZ;
    // float gpsLat;
    // float gpsLon;
    // float gpsAlt;
    // float gpsSpeed;
    // int gpsSats;
};

class Datalogger {
private:
    String filename;
    uint32_t _interval_ms;
    String _col_headers;
    uint8_t _num_cols;
    uint32_t _last_log_time;

public:
    Datalogger(uint32_t interval_ms);
    bool begin(const String& fn, String column_headers);
    bool create_log_file();
    bool append_row(sDataRow& data);
};