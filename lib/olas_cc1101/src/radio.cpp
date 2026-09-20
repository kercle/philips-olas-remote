#include <olas_cc1101/radio.h>

namespace olas_cc1101 {

RadioTransmitterResult::RadioTransmitterResult(bool _ok, int16_t _code)
    : success(_ok)
    , error_code(_code)
{
}

RadioTransmitterResult RadioTransmitterResult::ok()
{
    return RadioTransmitterResult(true, 0);
}

RadioTransmitterResult RadioTransmitterResult::err(int16_t code)
{
    return RadioTransmitterResult(false, code);
}

bool RadioTransmitterResult::is_err() const
{
    return !success;
}

bool RadioTransmitterResult::is_ok() const
{
    return success;
}

int16_t RadioTransmitterResult::code() const
{
    return error_code;
}

RadioTransmitterCC1101::RadioTransmitterCC1101(uint8_t pin_cs, uint8_t pin_gdo0, uint8_t pin_gdo2)
    : radio_module(pin_cs, pin_gdo0, RADIOLIB_NC, pin_gdo2)
    , radio(&radio_module)
    , rx_state(pin_gdo0)
    , initialized(false)
    , receiving(false)
{
}

RadioTransmitterResult RadioTransmitterCC1101::initialize()
{
    if (initialized) {
        return RadioTransmitterResult::ok();
    }

    int16_t state = 0;

    state = radio.begin(config::rf_frequency_mhz, config::raw_bitrate_kbps, 5.0, 100.0, 10, 16);
    if (state != RADIOLIB_ERR_NONE) {
        return RadioTransmitterResult::err(state);
    }

    state = radio.setOOK(true);
    if (state != RADIOLIB_ERR_NONE) {
        return RadioTransmitterResult::err(state);
    }

    //    We want the FIFO bytes to be transmitted literally.
    //    Promiscuous mode is useful here even though we're
    //    transmitting rather than receiving: in the current
    //    RadioLib CC1101 implementation it disables automatic
    //    sync-word insertion and disables CRC.
    state = radio.setPromiscuousMode(true);
    if (state != RADIOLIB_ERR_NONE) {
        return RadioTransmitterResult::err(state);
    }

    // No address byte.
    state = radio.disableAddressFiltering();
    if (state != RADIOLIB_ERR_NONE) {
        return RadioTransmitterResult::err(state);
    }

    auto ret = start_receiving();
    if (ret.is_err()) {
        return ret;
    }

    initialized = true;
    return RadioTransmitterResult::ok();
}

RadioTransmitterResult RadioTransmitterCC1101::start_receiving()
{
    // We don't have to check if we are initialized here,
    // since start_receiving is called by initialize
    // and transmit_burst stops and starts receiving again.

    if (receiving) {
        return RadioTransmitterResult::ok();
    }

    int16_t state = radio.receiveDirectAsync();
    if (state != RADIOLIB_ERR_NONE) {
        return RadioTransmitterResult::err(state);
    }

    pinMode(radio_module.getGpio(), INPUT);
    attachInterruptArg(
        digitalPinToInterrupt(radio_module.getIrq()),
        RadioRxState::handle_edge,
        static_cast<void*>(&rx_state),
        CHANGE);

    receiving = true;
    return RadioTransmitterResult::ok();
}

RadioTransmitterResult RadioTransmitterCC1101::stop_receiving()
{
    if (!receiving) {
        return RadioTransmitterResult::ok();
    }

    detachInterrupt(digitalPinToInterrupt(radio_module.getIrq()));

    int16_t state = radio.finishReceive();
    if (state != RADIOLIB_ERR_NONE) {
        return RadioTransmitterResult::err(state);
    }

    receiving = false;
    return RadioTransmitterResult::ok();
}

bool RadioTransmitterCC1101::is_initialized() const
{
    return initialized;
}

void RadioTransmitterCC1101::encode_bit(WaveformBuffer& waveform, bool bit)
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

void RadioTransmitterCC1101::encode_sync_signal(WaveformBuffer& waveform)
{
    // TODO: don't fail silently when waveform overfills
    // in particular if this code is reused for other
    // appliances.

    waveform.push_repeated(true, config::sync_on_bits);
    waveform.push_repeated(false, config::sync_off_bits);
}

void RadioTransmitterCC1101::build_waveform(WaveformBuffer& waveform, olas::Frame frame)
{
    encode_sync_signal(waveform);

    for (uint8_t i = 0; i < frame.bit_size(); ++i) {
        encode_bit(waveform, frame.get_frame_bit(i));
    }
}

bool RadioTransmitterCC1101::configure_transmit_mode()
{
    int16_t state = radio.packetMode();
    if (state != RADIOLIB_ERR_NONE) {
        return false;
    }

    state = radio.setPromiscuousMode(true);
    if (state != RADIOLIB_ERR_NONE) {
        return false;
    }

    /*
       Fixed packet length means there is no automatically
       transmitted length byte.

       The entire over-the-air payload is our 62-byte waveform.
    */
    state = radio.fixedPacketLengthMode(WaveformBuffer::capacity_bytes());
    return state == RADIOLIB_ERR_NONE;
}

bool RadioTransmitterCC1101::transmit_waveform(WaveformBuffer& waveform)
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
    // TODO: impl better solution later.
    delay(57);

    // CC1101 should now be in IDLE state.
    state = radio.finishTransmit();
    if (state != RADIOLIB_ERR_NONE) {
        return false;
    }

    return true;
}

bool RadioTransmitterCC1101::transmit(olas::Frame frm)
{
    return transmit_bursts(frm, 1);
}

bool RadioTransmitterCC1101::transmit_bursts(olas::Frame frm, uint8_t repeats)
{
    if (!is_initialized() || !repeats) {
        return false;
    }

    WaveformBuffer waveform;
    build_waveform(waveform, frm);

    if (stop_receiving().is_err()) {
        // If stop receiving didn't work
        // try to start it. In any case,
        // function wasn't successful.
        start_receiving();
        return false;
    }

    bool success = configure_transmit_mode();

    if (success) {
        for (uint8_t i = 0; i < repeats; ++i) {
            if (!transmit_waveform(waveform)) {
                success = false;
                break;
            }
        }
    }

    if (start_receiving().is_err()) {
        return false;
    }

    if (!success) {
        return false;
    }

    return true;
}

std::optional<olas::Frame> RadioTransmitterCC1101::receive_frame()
{
    auto frame_raw = rx_state.frame_data_queue.pop();

    if (!frame_raw.has_value()) {
        return std::nullopt;
    }

    return olas::Frame::from_data(frame_raw.value());
}

RadioRxState::RadioRxState(uint8_t pin_gdo0)
    : pin_gdo0(pin_gdo0)
{
}

void IRAM_ATTR RadioRxState::reset_state_machine(RadioRxState* self)
{
    self->current_state = RecvState::SeekingSyncOn;
    self->current_frame = 0;
    self->current_frame_size = 0;
}

void IRAM_ATTR RadioRxState::update_state(RadioRxState* self, EdgeClass cls)
{
    if (cls == EdgeClass::SyncOn) {
        self->current_frame = 0;
        self->current_frame_size = 0;
        self->current_state = RecvState::ExpectSyncOff;
        return;
    }

    switch (self->current_state) {
    case RecvState::SeekingSyncOn:
        // unreachable
        break;
    case RecvState::ExpectSyncOff:
        if (cls == EdgeClass::SyncOff) {
            self->current_state = RecvState::AwaitSymbolOn;
        } else {
            RadioRxState::reset_state_machine(self);
        }
        break;
    case RecvState::AwaitSymbolOn:
        if (cls == EdgeClass::ShortOn) {
            self->current_state = RecvState::ExpectLongOff;
        } else if (cls == EdgeClass::LongOn) {
            self->current_state = RecvState::ExpectShortOff;
        } else {
            RadioRxState::reset_state_machine(self);
        }
        break;
    case RecvState::ExpectLongOff:
        if (cls == EdgeClass::LongOff) {
            self->current_frame <<= 1;
            ++self->current_frame_size;

            self->current_state = RecvState::AwaitSymbolOn;
        } else {
            RadioRxState::reset_state_machine(self);
        }
        break;
    case RecvState::ExpectShortOff:
        if (cls == EdgeClass::ShortOff) {
            self->current_frame = (self->current_frame << 1) | 1;
            ++self->current_frame_size;

            self->current_state = RecvState::AwaitSymbolOn;
        } else {
            RadioRxState::reset_state_machine(self);
        }
        break;
    }

    if (self->current_frame_size == 40) {
        // From experimentation: The last bit is often incompletely
        // sent. We just stop here and assume it to be 0.

        self->frame_data_queue.push(self->current_frame << 1);
        RadioRxState::reset_state_machine(self);
    }
}

bool IRAM_ATTR is_in_timing_window(uint32_t window_center, uint32_t value)
{
    const uint32_t lower_bound = window_center - config::timing_window_width / 2;
    const uint32_t upper_bound = window_center + config::timing_window_width / 2;

    return lower_bound <= value && value <= upper_bound;
}

void IRAM_ATTR RadioRxState::handle_edge(void* self_raw)
{
    auto self = static_cast<RadioRxState*>(self_raw);

    uint32_t now = micros();
    auto duration = now - self->last_edge_time;
    self->last_edge_time = now;

    auto gpio_level_after_edge = digitalRead(self->pin_gdo0);
    auto carrier_on = !gpio_level_after_edge;

    EdgeClass cls = EdgeClass::Unknown;
    if (is_in_timing_window(config::median_short_pulse_duration, duration)) {
        cls = carrier_on
            ? EdgeClass::ShortOn
            : EdgeClass::ShortOff;
    } else if (is_in_timing_window(config::median_long_pulse_duration, duration)) {
        cls = carrier_on
            ? EdgeClass::LongOn
            : EdgeClass::LongOff;
    } else if (is_in_timing_window(config::median_sync_on_duration, duration)) {
        cls = carrier_on
            ? EdgeClass::SyncOn
            : EdgeClass::Unknown;
    } else if (is_in_timing_window(config::median_sync_off_duration, duration)) {
        cls = carrier_on
            ? EdgeClass::Unknown
            : EdgeClass::SyncOff;
    }

    RadioRxState::update_state(self, cls);
}

}
