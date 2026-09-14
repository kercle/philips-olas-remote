#pragma once

#include <RadioLib.h>

#include <config.h>
#include <olas/protocol.h>

namespace olas {

template <unsigned N_BYTES>
class BitSequence {
    unsigned bit_idx;
    uint8_t data[N_BYTES];

public:
    BitSequence()
        : bit_idx(0)
    {
        memset(data, 0, N_BYTES);
    }

    bool push(bool bit)
    {
        if (bit_idx >= N_BYTES * 8) {
            // no space left
            return false;
        }

        unsigned byte_idx = bit_idx >> 3;
        unsigned rel_bit_idx = 7 - (bit_idx & 7);

        if (bit) {
            data[byte_idx] |= 1 << rel_bit_idx;
        }

        ++bit_idx;

        return true;
    }

    bool push_repeated(bool bit, unsigned count)
    {
        for (unsigned i = 0; i < count; ++i) {
            if (!push(bit)) {
                return false;
            }
        }

        return true;
    }

    const uint8_t* get_raw() const
    {
        return data;
    }
};

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

class RadioTransmitter {
    FrameBuilder& frame_builder;

    Module radio_module;
    CC1101 radio;

    bool initialized;

    void encode_bit(BitSequence<WAVEFORM_BYTES>& waveform, bool bit);
    void encode_sync_signal(BitSequence<WAVEFORM_BYTES>& waveform);
    void build_waveform(BitSequence<WAVEFORM_BYTES>& waveform, Command cmd);

    bool transmit_waveform(BitSequence<WAVEFORM_BYTES>& waveform);

public:
    RadioTransmitter(FrameBuilder& frame_builder);
    RadioTransmitterInitResult initialize();

    bool transmit(Command cmd);
    bool transmit_bursts(Command cmd, uint8_t repeats);
};

}
