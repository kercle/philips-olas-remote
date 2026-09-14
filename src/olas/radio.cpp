#include <olas/radio.h>

namespace olas {

RadioTransmitter::RadioTransmitter(FrameBuilder& frame_builder, CC1101 radio)
    : frame_builder(frame_builder)
    , radio(radio)
{
}

void RadioTransmitter::encode_bit(BitSequence<WAVEFORM_BYTES>& waveform, bool bit)
{
    if (bit) {
        waveform.push_repeated(true, LONG_BITS);
        waveform.push_repeated(false, SHORT_BITS);
    } else {
        waveform.push_repeated(true, SHORT_BITS);
        waveform.push_repeated(false, LONG_BITS);
    }
}

void RadioTransmitter::encode_sync_signal(BitSequence<WAVEFORM_BYTES>& waveform)
{
    waveform.push_repeated(true, SYNC_ON_BITS);
    waveform.push_repeated(false, SYNC_OFF_BITS);
}

void RadioTransmitter::build_waveform(BitSequence<WAVEFORM_BYTES>& waveform, Command cmd)
{
    encode_sync_signal(waveform);
    auto frame = frame_builder.build(cmd);

    for (uint8_t i = 0; i < frame.bit_size(); ++i) {
        encode_bit(waveform, frame.get_frame_bit(i));
    }
}

bool RadioTransmitter::transmit_waveform(BitSequence<WAVEFORM_BYTES>& waveform)
{
    int16_t state = radio.startTransmit(waveform.get_raw(), WAVEFORM_BYTES);
    if (state != RADIOLIB_ERR_NONE) {
        return false;
    }

    /*
    Safety delay: 62 bytes * 8 = 496 bits
    496 / 9323 bits/s ~= 53.2 ms
    This mean that if we wait 57 ms, we cannot flood the CC1101
    With frames essentially transmitting garbage.
    */
    delay(57);

    // CC1101 should now be in IDLE state.
    state = radio.finishTransmit();
    return state == RADIOLIB_ERR_NONE;
}

bool RadioTransmitter::transmit(Command cmd)
{
    BitSequence<WAVEFORM_BYTES> waveform;
    build_waveform(waveform, cmd);
    return transmit_waveform(waveform);
}

bool RadioTransmitter::transmit_bursts(Command cmd, uint8_t repeats)
{
    BitSequence<WAVEFORM_BYTES> waveform;
    build_waveform(waveform, cmd);

    for (uint8_t i = 0; i < repeats; ++i) {
        if (!transmit_waveform(waveform)) {
            return false;
        }
    }

    return true;
}

}
