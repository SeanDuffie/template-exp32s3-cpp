#pragma once
#include "debug.h"
#include <LittleFS.h>
#include <SD.h>
#include <sdcard.h>
#include "core/sensor_stats.h"

struct sDataRow {
    String timestamp;
    unsigned long uptime;
    // float tempC;
    // float tempF;
    // float humidity;
    // float pressure;
};

class Datalogger {
private:
    String filename;
    uint32_t _interval_ms;
    String _col_headers;
    uint8_t _num_cols;
    uint32_t _last_log_time;
    bool _use_sd;

public:
    Datalogger(uint32_t interval_ms);
    bool setup_logfile(const String& fn, String column_headers, bool use_sd);
    bool append_row(sDataRow& data);
};