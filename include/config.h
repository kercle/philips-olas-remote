#pragma once

#define PIN_CS 15
#define PIN_GDO0 4
#define PIN_GDO2 5

namespace config {

// Radio configuration
constexpr float rf_frequency_mhz = 433.920f;
constexpr float raw_bitrate_kbps = 9.323f;

constexpr unsigned sync_on_bits = 69;
constexpr unsigned sync_off_bits = 10;

constexpr unsigned short_bits = 3;
constexpr unsigned long_bits = 7;

constexpr unsigned waveform_bytes = 62;

// Web server configuration
static const char hostname[] = "philips-olas-remote";
constexpr unsigned task_execution_time_gap = 50; // milliseconds

}
