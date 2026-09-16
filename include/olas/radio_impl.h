#pragma once

#include <RadioLib.h>

#include <config.h>
#include <olas/bit_sequence.h>
#include <olas/protocol.h>

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

// This class contains all the moving parts for communicating
// via CC1101 chips.
// Do not use this class explicitly, as this does not register
// an ISR for receiving data. Instead use the class RadioTransmitter
// with templates being the the required pins to communicate with
// the chip.
class RadioTransmitterImpl {
    FrameBuilder* frame_builder;

    Module& radio_module;
    CC1101& radio;

    bool initialized;

    void encode_bit(WaveformBuffer& waveform, bool bit);
    void encode_sync_signal(WaveformBuffer& waveform);
    void build_waveform(WaveformBuffer& waveform, Command cmd);

    bool transmit_waveform(WaveformBuffer& waveform);

public:
    RadioTransmitterImpl(Module& radio_module, CC1101& radio);

    RadioTransmitterInitResult initialize(FrameBuilder* frame_builder);
    bool is_initialized() const;

    bool transmit(Command cmd);
    bool transmit_bursts(Command cmd, uint8_t repeats);
};

}
