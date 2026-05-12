#include "core/datalogger.h"

Datalogger::Datalogger(uint32_t interval_ms) : _interval_ms(interval_ms) {
    // Initialize other members if needed
    this->filename = "";
    this->_col_headers = "";
    this->_num_cols = -1;
    this->_last_log_time = -1;
    this->_use_sd = false;
}

bool Datalogger::setup_logfile(const String& fn, String column_headers, bool use_sd) {
    this->filename = fn;
    this->_col_headers = column_headers;
    this->_use_sd = use_sd;
    this->_num_cols = column_headers.length() - column_headers.lastIndexOf(',') > 0 ? column_headers.lastIndexOf(',') : 0;
    if (this->_use_sd) {
        // if (!SD.exists(this->filename)) {
        //     File file = SD.open(this->filename, FILE_WRITE);
        //     if (file) {
        //         file.print("Timestamp,Uptime_s");

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
                file.print("Timestamp,Uptime_s");

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
    // // 1. Tell all nodes to read hardware and run math
    // for (int i = 0; i < NODE_COUNT; i++) {
    //     activeNodes[i]->sample();
    // }

    // // 2. Check timer
    if (millis() - this->_last_log_time < this->_interval_ms) { return false; }
    this->_last_log_time = millis();

    // 3. Construct the CSV row safely without String objects
    char row_buffer[512];
    size_t offset = snprintf(row_buffer, sizeof(row_buffer), "%s,%u", data.timestamp.c_str(), data.uptime);

    // for (int i = 0; i < NODE_COUNT; i++) {
    //     if (offset < sizeof(row_buffer) - 1) {
    //         row_buffer[offset++] = ','; // Add comma
    //         offset += activeNodes[i]->writeData(row_buffer + offset, sizeof(row_buffer) - offset);
    //     }
    // }

    // 4. Write to File
    if (this->_use_sd) {
        // // Write to SD Card
        // if (!SDCard.appendLog(this->filename.c_str(), row_buffer)) {
        //     debug_println("Failed to append to SD card.");
        //     return false;
        // }
        debug_println("ERROR: SD card not implemented for logging yet.");
    } else {
        // Write to LittleFS
        File file = LittleFS.open("/sensor_log.csv", FILE_APPEND);
        if (file) {
            file.println(row_buffer);
            file.close();
        }
        else {
            debug_println("Failed to open log file for appending.");
            return false;
        }
    }

    return true;

    // // 5. Reset all math objects
    // for (int i = 0; i < NODE_COUNT; i++) {
    //     activeNodes[i]->reset();
    // }
}