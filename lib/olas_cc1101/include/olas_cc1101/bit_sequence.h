#pragma once

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

    constexpr static unsigned capacity_bytes() {
        return N_BYTES;
    }
};
