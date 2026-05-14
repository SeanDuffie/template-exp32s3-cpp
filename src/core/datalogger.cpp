#include "core/datalogger.h"

Datalogger::Datalogger(uint32_t interval_ms) : _interval_ms(interval_ms) {
    // Initialize other members if needed
    this->filename = "";
    this->_col_headers = "";
    this->_num_cols = -1;
    this->_last_log_time = -1;
    this->_use_sd = false;
}

bool Datalogger::begin(const String& fn, String column_headers, bool use_sd) {
    this->filename = fn;
    this->_col_headers = column_headers;
    this->_use_sd = use_sd;
    this->_num_cols = column_headers.length() - column_headers.lastIndexOf(',') > 0 ? column_headers.lastIndexOf(',') : 0;
    if (this->_use_sd) {
        // if (!SD.exists(this->filename)) {
        //     File file = SD.open(this->filename, FILE_WRITE);
        //     if (file) {
        //         file.print("Timestamp,Uptime_ms");

        //         // // Iterate to build the header string dynamically on boot
        //         // for (int i = 0; i < this->sensorList.length(); i++) {
        //         //     file.print(",");
        //         //     file.print(activeNodes[i]->getHeaders());
        //         // }

        //         file.println();
        //         file.close();
        //     }
        //     else {
        //         debug_println("Failed to create log file.");
        //         return false;
        //     }
        // }
        // else {
        //     debug_println("Log file already exists. Appending new data.");
        // }
        debug_println("ERROR: SD card not implemented for logging yet.");
    } else {
        if (!LittleFS.exists(this->filename)) {
            File file = LittleFS.open(this->filename, FILE_WRITE);
            if (file) {
                file.print("Timestamp,Uptime_ms");

                // // Iterate to build the header string dynamically on boot
                // for (int i = 0; i < this->sensorList.length(); i++) {
                //     file.print(",");
                //     file.print(activeNodes[i]->getHeaders());
                // }

                file.println();
                file.close();
            }
            else {
                debug_println("Failed to create log file.");
                return false;
            }
        }
        else {
            debug_println("Log file already exists. Appending new data.");
        }
    }
    return true;
}

// Call this every 1000ms
bool Datalogger::append_row(sDataRow& data) {
    // Check timer
    if (millis() - this->_last_log_time < this->_interval_ms) { return true; }

    // Construct the CSV row safely without String objects
    char row_buffer[512];
    size_t offset = snprintf(row_buffer, sizeof(row_buffer), "%s,%u", data.timestamp.c_str(), data.uptime);

    // Pass the filepath and the formatted CSV row.
    bool success = FileSystem.appendLog(this->filename.c_str(), row_buffer);

    if (success) {
        this->_last_log_time = millis();
    }

    return success;
}