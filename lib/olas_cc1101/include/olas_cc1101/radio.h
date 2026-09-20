#pragma once

#include <RadioLib.h>

#include <config.h>

#include <olas/radio.h>

#include <olas_cc1101/bit_sequence.h>
#include <olas_cc1101/config.h>
#include <olas_cc1101/ring_buffer.h>

namespace olas_cc1101 {

class RadioTransmitterResult {
    bool success;
    int16_t error_code;

    RadioTransmitterResult(bool success, int16_t error_code);

public:
    static RadioTransmitterResult ok();
    static RadioTransmitterResult err(int16_t error_code);

    bool is_err() const;
    bool is_ok() const;

    int16_t code() const;
};

using WaveformBuffer = containers::BitSequence<config::waveform_bytes>;

constexpr size_t ring_buffer_size = 64;

// State machine for receiving signals:
// Transitions:
//   SeekingSyncOn → {ExpectSyncOff, SeekingSyncOn}
//   ExpectSyncOff → {AwaitSymbolOn, SeekingSyncOn}
//   AwaitSymbolOn → {ExpectLongOff, ExpectShortOff, SeekingSyncOn}
//   ExpectLongOff → {AwaitSymbolOn, SeekingSyncOn}
//   ExpectShortOff → {AwaitSymbolOn, SeekingSyncOn}
enum class RecvState : uint8_t {
    SeekingSyncOn,
    ExpectSyncOff,
    AwaitSymbolOn,
    ExpectLongOff,
    ExpectShortOff,
};

enum class EdgeClass : uint8_t {
    ShortOn,
    ShortOff,
    LongOn,
    LongOff,
    SyncOn,
    SyncOff,
    Unknown,
};

struct RadioRxState {
    uint8_t pin_gdo0;

    volatile uint32_t last_edge_time = 0;
    volatile RecvState current_state = RecvState::SeekingSyncOn;
    volatile uint64_t current_frame = 0;
    volatile uint8_t current_frame_size = 0;
    containers::RingBuffer<uint64_t, ring_buffer_size> frame_data_queue;

    static void IRAM_ATTR reset_state_machine(RadioRxState* self);
    static void IRAM_ATTR update_state(RadioRxState* self, EdgeClass cls);
    static void IRAM_ATTR handle_edge(void* self_raw);

    RadioRxState(uint8_t pin_gdo0);
};

class RadioTransmitterCC1101 : public olas::RadioTransmitter {
    Module radio_module;
    CC1101 radio;

    RadioRxState rx_state;

    bool initialized;
    bool receiving;

    RadioTransmitterResult configure_tx_mode();

    void encode_bit(WaveformBuffer& waveform, bool bit);
    void encode_sync_signal(WaveformBuffer& waveform);
    void build_waveform(WaveformBuffer& waveform, olas::Frame frame);

    bool configure_transmit_mode();
    bool transmit_waveform(WaveformBuffer& waveform);

    RadioTransmitterResult start_receiving();
    RadioTransmitterResult stop_receiving();

public:
    RadioTransmitterCC1101(uint8_t pin_cs, uint8_t pin_gdo0, uint8_t pin_gdo2);

    RadioTransmitterResult initialize();
    bool is_initialized() const;

    virtual bool transmit(olas::Frame frm);
    virtual bool transmit_bursts(olas::Frame frm, uint8_t repeats);

    virtual std::optional<olas::Frame> receive_frame();
};

}
