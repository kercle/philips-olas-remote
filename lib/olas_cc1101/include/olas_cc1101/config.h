#pragma once

namespace olas_cc1101 {
namespace config {
    // General settings

    constexpr float rf_frequency_mhz = 434.0f;
    constexpr float raw_bitrate_kbps = 9.323f;

    constexpr unsigned sync_on_bits = 69;
    constexpr unsigned sync_off_bits = 10;

    constexpr unsigned short_bits = 3;
    constexpr unsigned long_bits = 7;

    // Transmission sett

    constexpr unsigned waveform_bytes = 62;

    // Receiver settings

    constexpr unsigned timing_window_width = 140; // µs
    constexpr unsigned median_short_pulse_duration = 340; // µs
    constexpr unsigned median_long_pulse_duration = 740; // µs
    constexpr unsigned median_sync_on_duration = 7400; // µs
    constexpr unsigned median_sync_off_duration = 1090; // µs
}
}
