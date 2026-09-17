#pragma once

namespace config {

//# Radio configuration

//## General settings

constexpr unsigned char pin_cs = 15;
constexpr unsigned char pin_gdo0 = 4;
constexpr unsigned char pin_gdo2 = 5;

constexpr float rf_frequency_mhz = 434.0f;
constexpr float raw_bitrate_kbps = 9.323f;

constexpr unsigned sync_on_bits = 69;
constexpr unsigned sync_off_bits = 10;

constexpr unsigned short_bits = 3;
constexpr unsigned long_bits = 7;

constexpr unsigned waveform_bytes = 62;

//## Receiver config
constexpr unsigned timing_window_width = 140; // µs
constexpr unsigned median_short_pulse_duration = 340;  // µs
constexpr unsigned median_long_pulse_duration = 740; // µs
constexpr unsigned median_sync_on_duration = 7400; // µs
constexpr unsigned median_sync_off_duration = 1090; // µs


// Web server configuration
static const char hostname[] = "philips-olas-remote";
constexpr unsigned task_execution_time_gap = 50; // milliseconds

}
