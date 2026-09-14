#pragma once

#define PIN_CS 15
#define PIN_GDO0 4
#define PIN_GDO2 5

// Radio configuration
constexpr float RF_FREQUENCY_MHZ = 433.920f;
constexpr float RAW_BITRATE_KBPS = 9.323f;

constexpr unsigned SYNC_ON_BITS  = 69;
constexpr unsigned SYNC_OFF_BITS = 10;

constexpr unsigned SHORT_BITS = 3;
constexpr unsigned LONG_BITS  = 7;

constexpr unsigned WAVEFORM_BYTES = 62;

// Web server configuration
constexpr unsigned TASK_EXECUTION_WAIT_PERIOD = 50; // milliseconds
