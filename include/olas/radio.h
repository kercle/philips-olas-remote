#pragma once

#include <RadioLib.h>

#include <config.h>
#include <olas/protocol.h>
#include <olas/bit_sequence.h>

namespace olas {

class RadioTransmitterInitResult {
    bool _ok;
    int16_t _code;

    RadioTransmitterInitResult(bool _ok, int16_t _code);

public:
    static RadioTransmitterInitResult ok();
    static RadioTransmitterInitResult err(int16_t code);

    bool is_err() const;
    int16_t code() const;
};

typedef BitSequence<config::waveform_bytes> WaveformBuffer;

class RadioTransmitter {
    FrameBuilder& frame_builder;

    Module radio_module;
    CC1101 radio;

    bool initialized;

    void encode_bit(WaveformBuffer& waveform, bool bit);
    void encode_sync_signal(WaveformBuffer& waveform);
    void build_waveform(WaveformBuffer& waveform, Command cmd);

    bool transmit_waveform(WaveformBuffer& waveform);

public:
    RadioTransmitter(FrameBuilder& frame_builder);
    RadioTransmitterInitResult initialize();

    bool transmit(Command cmd);
    bool transmit_bursts(Command cmd, uint8_t repeats);
};

}
