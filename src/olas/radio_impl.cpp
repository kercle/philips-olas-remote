#include <olas/radio_impl.h>

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

RadioTransmitterImpl::RadioTransmitterImpl(Module& radio_module, CC1101& radio)
    : frame_builder(nullptr)
    , radio_module(radio_module)
    , radio(radio)
    , initialized(false)
{
}

RadioTransmitterInitResult RadioTransmitterImpl::initialize(FrameBuilder* frame_builder)
{
    if (initialized) {
        return RadioTransmitterInitResult::ok();
    }

    this->frame_builder = frame_builder;

    int16_t state = 0;

    state = radio.begin(config::rf_frequency_mhz, config::raw_bitrate_kbps, 5.0, 100.0, 10, 16);
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
    state = radio.fixedPacketLengthMode(WaveformBuffer::capacity_bytes());
    if (state != RADIOLIB_ERR_NONE) {
        return RadioTransmitterInitResult::err(state);
    }

    initialized = true;
    return RadioTransmitterInitResult::ok();
}

bool RadioTransmitterImpl::is_initialized() const {
    return initialized;
}

void RadioTransmitterImpl::encode_bit(WaveformBuffer& waveform, bool bit)
{
    // TODO: don't fail silently when waveform overfills
    // in particular if this code is reused for other
    // appliances.

    if (bit) {
        waveform.push_repeated(true, config::long_bits);
        waveform.push_repeated(false, config::short_bits);
    } else {
        waveform.push_repeated(true, config::short_bits);
        waveform.push_repeated(false, config::long_bits);
    }
}

void RadioTransmitterImpl::encode_sync_signal(WaveformBuffer& waveform)
{
    // TODO: don't fail silently when waveform overfills
    // in particular if this code is reused for other
    // appliances.

    waveform.push_repeated(true, config::sync_on_bits);
    waveform.push_repeated(false, config::sync_off_bits);
}

void RadioTransmitterImpl::build_waveform(WaveformBuffer& waveform, Command cmd)
{
    encode_sync_signal(waveform);
    auto frame = frame_builder->build(cmd);

    for (uint8_t i = 0; i < frame.bit_size(); ++i) {
        encode_bit(waveform, frame.get_frame_bit(i));
    }
}

bool RadioTransmitterImpl::transmit_waveform(WaveformBuffer& waveform)
{
    int16_t state = radio.startTransmit(
        waveform.get_raw(),
        WaveformBuffer::capacity_bytes());

    if (state != RADIOLIB_ERR_NONE) {
        return false;
    }

    // Safety delay: 62 bytes * 8 = 496 bits
    // 496 / 9323 bits/s ~= 53.2 ms
    // This mean that if we wait 57 ms, we cannot flood the CC1101
    // With frames essentially transmitting garbage.
    // TODO: maybe something more generalizable to async envs
    // e.g. a function that checks if this time has passed and
    // lets the caller decide if to delay or not.
    delay(57);

    // CC1101 should now be in IDLE state.
    state = radio.finishTransmit();
    return state == RADIOLIB_ERR_NONE;
}

bool RadioTransmitterImpl::transmit(Command cmd)
{
    return transmit_bursts(cmd, 1);
}

bool RadioTransmitterImpl::transmit_bursts(Command cmd, uint8_t repeats)
{
    if (!initialized || !repeats) {
        return false;
    }

    WaveformBuffer waveform;
    build_waveform(waveform, cmd);

    for (uint8_t i = 0; i < repeats; ++i) {
        if (!transmit_waveform(waveform)) {
            return false;
        }
    }

    frame_builder->advance_frame_counter();
    return true;
}

}
