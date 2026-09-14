#include <olas/radio.h>

namespace olas {

RadioTransmitterInitResult::RadioTransmitterInitResult(bool _ok, int16_t _code)
    : _ok(_ok)
    , _code(_code)
{
}

RadioTransmitterInitResult RadioTransmitterInitResult::ok()
{
    return RadioTransmitterInitResult(true, 0);
}

RadioTransmitterInitResult RadioTransmitterInitResult::err(int16_t code)
{
    return RadioTransmitterInitResult(false, code);
}

bool RadioTransmitterInitResult::is_err() const
{
    return !_ok;
}

int16_t RadioTransmitterInitResult::code() const
{
    return _code;
}

RadioTransmitter::RadioTransmitter(FrameBuilder& frame_builder)
    : frame_builder(frame_builder)
    , radio_module(PIN_CS, PIN_GDO0, RADIOLIB_NC, PIN_GDO2)
    , radio(&radio_module)
    , initialized(false)
{
}

RadioTransmitterInitResult RadioTransmitter::initialize()
{
    if (initialized) {
        return RadioTransmitterInitResult::ok();
    }

    int16_t state = 0;

    state = radio.begin(RF_FREQUENCY_MHZ, RAW_BITRATE_KBPS, 5.0, 325.0, 10, 16);
    if (state != RADIOLIB_ERR_NONE) {
        return RadioTransmitterInitResult::err(state);
    }

    state = radio.setOOK(true);
    if (state != RADIOLIB_ERR_NONE) {
        return RadioTransmitterInitResult::err(state);
    }

    //    We want the FIFO bytes to be transmitted literally.
    //    Promiscuous mode is useful here even though we're
    //    transmitting rather than receiving: in the current
    //    RadioLib CC1101 implementation it disables automatic
    //    sync-word insertion and disables CRC.
    state = radio.setPromiscuousMode(true);
    if (state != RADIOLIB_ERR_NONE) {
        return RadioTransmitterInitResult::err(state);
    }

    // No address byte.
    state = radio.disableAddressFiltering();
    if (state != RADIOLIB_ERR_NONE) {
        return RadioTransmitterInitResult::err(state);
    }

    /*
       Fixed packet length means there is no automatically
       transmitted length byte.

       The entire over-the-air payload is our 62-byte waveform.
    */
    state = radio.fixedPacketLengthMode(WAVEFORM_BYTES);
    if (state != RADIOLIB_ERR_NONE) {
        return RadioTransmitterInitResult::err(state);
    }

    initialized = true;
    return RadioTransmitterInitResult::ok();
}

void RadioTransmitter::encode_bit(BitSequence<WAVEFORM_BYTES>& waveform, bool bit)
{
    // TODO: don't fail silently when waveform overfills
    // in particular if this code is reused for other
    // appliances.

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
    // TODO: don't fail silently when waveform overfills
    // in particular if this code is reused for other
    // appliances.

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

    // Safety delay: 62 bytes * 8 = 496 bits
    // 496 / 9323 bits/s ~= 53.2 ms
    // This mean that if we wait 57 ms, we cannot flood the CC1101
    // With frames essentially transmitting garbage.
    delay(57);

    // CC1101 should now be in IDLE state.
    state = radio.finishTransmit();
    return state == RADIOLIB_ERR_NONE;
}

bool RadioTransmitter::transmit(Command cmd)
{
    return transmit_bursts(cmd, 1);
}

bool RadioTransmitter::transmit_bursts(Command cmd, uint8_t repeats)
{
    if (!initialized || !repeats) {
        return false;
    }

    BitSequence<WAVEFORM_BYTES> waveform;
    build_waveform(waveform, cmd);

    for (uint8_t i = 0; i < repeats; ++i) {
        if (!transmit_waveform(waveform)) {
            return false;
        }
    }

    frame_builder.advance_frame_counter();
    return true;
}

}
