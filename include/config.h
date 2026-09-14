#pragma once

#define PIN_CS 15
#define PIN_GDO0 4
#define PIN_GDO2 5

// Radio configuration
constexpr float RF_FREQUENCY_MHZ = 433.920f;
constexpr float RAW_BITRATE_KBPS = 9.323f;

static const uint16_t SYNC_ON_BITS  = 69;
static const uint16_t SYNC_OFF_BITS = 10;

static const uint8_t SHORT_BITS = 3;
static const uint8_t LONG_BITS  = 7;

constexpr uint8_t WAVEFORM_BYTES = 62;
