#include "core/datalogger.h"

Datalogger::Datalogger(uint32_t interval_ms) : _interval_ms(interval_ms) {
    // Initialize other members if needed
    this->filename = "";
    this->_col_headers = "";
    this->_num_cols = -1;
    this->_last_log_time = -1;
}

bool Datalogger::begin(const String& fn, String column_headers) {
    this->filename = fn;
    this->_col_headers = column_headers;
    this->_num_cols = column_headers.length() - column_headers.lastIndexOf(',') > 0 ? column_headers.lastIndexOf(',') : 0;

    bool result = create_log_file();

    return result;
}

bool Datalogger::create_log_file() {
    fs::FS* targetFS = FileSystem.getDrive("sd");
    if (targetFS == nullptr) {
        debug_println("Datalogger: SD card requested but currently unavailable. Defaulting to LittleFS.");
        targetFS = FileSystem.getDrive("littlefs");
    }

    bool result = false;

    if (targetFS != nullptr) {
        if (!targetFS->exists(this->filename)) {
            result = FileSystem.appendLog(this->filename.c_str(), this->_col_headers.c_str());
            debug_println("Datalogger: Created new log file with headers.");
        } else {
            debug_println("Datalogger: Log file already exists. Appending new data.");
            result = true; // File exists, so we can append to it
        }
    } else {
        debug_println("Datalogger: ERROR: No valid filesystem found.");
    }

    return result;
}

// Call this every 1000ms
bool Datalogger::append_row(sDataRow& data) {
    // Check timer
    if (millis() - this->_last_log_time < this->_interval_ms) { return true; }

    create_log_file(); // Ensure the file exists before appending

    // MARK: Modify for enabled sensors
    // Construct the CSV row safely without String objects
    char row_buffer[512];
    size_t offset = snprintf(
        row_buffer,
        sizeof(row_buffer),
        "%s,%lu,%.2f",//,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d",
        data.timestamp.c_str(),
        data.uptime,
        data.lux
        // data.soilMoisture,
        // data.soilMoistureADC,
        // data.soilTempC,
        // data.soilTempF,
        // data.airTempC,
        // data.AirTempF,
        // data.humidity,
        // data.pressure,
        // data.accelX,
        // data.accelY,
        // data.accelZ,
        // data.gyroX,
        // data.gyroY,
        // data.gyroZ,
        // data.gpsLat,
        // data.gpsLon,
        // data.gpsAlt,
        // data.gpsSpeed,
        // data.gpsSats
    );

    // Pass the filepath and the formatted CSV row.
    bool success = FileSystem.appendLog(this->filename.c_str(), row_buffer);

    if (success) {
        this->_last_log_time = millis();
    }

    return success;
}