#pragma once

#include <RadioLib.h>

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

    void push(bool bit)
    {
        if (bit_idx >= N_BYTES * 8) {
            // no space left
            return;
        }

        unsigned byte_idx = bit_idx >> 3;
        unsigned rel_bit_idx = 7 - (bit_idx & 7);

        if (bit) {
            data[byte_idx] |= 1 << rel_bit_idx;
        }

        ++bit_idx;
    }

    void push_repeated(bool bit, unsigned count)
    {
        for (unsigned i = 0; i < count; ++i) {
            push(bit);
        }
    }
};

class RadioTransmitter {
    FrameBuilder& frame_builder;
    CC1101 radio;

public:
    RadioTransmitter(FrameBuilder& frame_builder, CC1101 radio);
};

}
