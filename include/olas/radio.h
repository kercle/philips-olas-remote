#pragma once

#include <RadioLib.h>

#include <config.h>
#include <olas/bit_sequence.h>
#include <olas/protocol.h>
#include <olas/radio_impl.h>
#include <olas/ring_buffer.h>

namespace olas {

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

template <uint8_t PIN_CS, uint8_t PIN_GDO0, uint8_t PIN_GDO2>
class RadioTransmitter {
    Module radio_module;
    CC1101 radio;
    RadioTransmitterImpl impl;

    bool receiving;

    inline static volatile uint32_t last_edge_time = 0;
    inline static volatile RecvState current_state = RecvState::SeekingSyncOn;
    inline static volatile uint64_t current_frame = 0;
    inline static volatile uint8_t current_frame_size = 0;
    inline static volatile RingBuffer<uint64_t, ring_buffer_size> frame_data_queue;

    static constexpr uint8_t cc1101_reg_iocfg0 = 0x02;
    static constexpr uint8_t cc1101_reg_pktctrl0 = 0x08;

    RadioTransmitter()
        : radio_module(PIN_CS, PIN_GDO0, RADIOLIB_NC, PIN_GDO2)
        , radio(&radio_module)
        , impl(radio_module, radio)
        , receiving(false)
    {
    }

    void write_raw_register(uint8_t addr, uint8_t value)
    {
        digitalWrite(PIN_CS, LOW);
        SPI.transfer(addr);
        SPI.transfer(value);
        digitalWrite(PIN_CS, HIGH);
    }

    static void IRAM_ATTR reset_state_machine()
    {
        current_state = RecvState::SeekingSyncOn;
        current_frame = 0;
        current_frame_size = 0;
    }

    static void IRAM_ATTR update_state(EdgeClass cls)
    {
        if (cls == EdgeClass::SyncOn) {
            current_frame = 0;
            current_frame_size = 0;
            current_state = RecvState::ExpectSyncOff;
            return;
        }

        switch (current_state) {
        case RecvState::SeekingSyncOn:
            // unreachable
            break;
        case RecvState::ExpectSyncOff:
            if (cls == EdgeClass::SyncOff) {
                current_state = RecvState::AwaitSymbolOn;
            } else {
                RadioTransmitter<PIN_CS, PIN_GDO0, PIN_GDO2>::reset_state_machine();
            }
            break;
        case RecvState::AwaitSymbolOn:
            if (cls == EdgeClass::ShortOn) {
                current_state = RecvState::ExpectLongOff;
            } else if (cls == EdgeClass::LongOn) {
                current_state = RecvState::ExpectShortOff;
            } else {
                RadioTransmitter<PIN_CS, PIN_GDO0, PIN_GDO2>::reset_state_machine();
            }
            break;
        case RecvState::ExpectLongOff:
            if (cls == EdgeClass::LongOff) {
                current_frame <<= 1;
                ++current_frame_size;

                current_state = RecvState::AwaitSymbolOn;
            } else {
                RadioTransmitter<PIN_CS, PIN_GDO0, PIN_GDO2>::reset_state_machine();
            }
            break;
        case RecvState::ExpectShortOff:
            if (cls == EdgeClass::ShortOff) {
                current_frame = (current_frame << 1) | 1;
                ++current_frame_size;

                current_state = RecvState::AwaitSymbolOn;
            } else {
                RadioTransmitter<PIN_CS, PIN_GDO0, PIN_GDO2>::reset_state_machine();
            }
            break;
        }

        if (current_frame_size == 40) {
            // From experimentation: The last bit is often incompletely
            // sent. We just stop here and assume it to be 0.

            frame_data_queue.push(current_frame << 1);
            RadioTransmitter<PIN_CS, PIN_GDO0, PIN_GDO2>::reset_state_machine();
        }
    }

    static void IRAM_ATTR handle_edge()
    {
        uint32_t now = micros();
        auto duration = now - last_edge_time;
        last_edge_time = now;

        auto gpio_level_after_edge = digitalRead(PIN_GDO0);
        auto carrier_on = !gpio_level_after_edge;

        constexpr uint32_t short_lower_bound = config::median_short_pulse_duration - config::timing_window_width / 2;
        constexpr uint32_t short_upper_bound = config::median_short_pulse_duration + config::timing_window_width / 2;

        constexpr uint32_t long_lower_bound = config::median_long_pulse_duration - config::timing_window_width / 2;
        constexpr uint32_t long_upper_bound = config::median_long_pulse_duration + config::timing_window_width / 2;

        constexpr uint32_t sync_on_lower_bound = config::median_sync_on_duration - config::timing_window_width / 2;
        constexpr uint32_t sync_on_upper_bound = config::median_sync_on_duration + config::timing_window_width / 2;

        constexpr uint32_t sync_off_lower_bound = config::median_sync_off_duration - config::timing_window_width / 2;
        constexpr uint32_t sync_off_upper_bound = config::median_sync_off_duration + config::timing_window_width / 2;

        EdgeClass cls = EdgeClass::Unknown;
        if (short_lower_bound <= duration && duration <= short_upper_bound) {
            cls = carrier_on
                ? EdgeClass::ShortOn
                : EdgeClass::ShortOff;
        } else if (long_lower_bound <= duration && duration <= long_upper_bound) {
            cls = carrier_on
                ? EdgeClass::LongOn
                : EdgeClass::LongOff;
        } else if (sync_on_lower_bound <= duration && duration <= sync_on_upper_bound) {
            cls = carrier_on
                ? EdgeClass::SyncOn
                : EdgeClass::Unknown;
        } else if (sync_off_lower_bound <= duration && duration <= sync_off_upper_bound) {
            cls = carrier_on
                ? EdgeClass::Unknown
                : EdgeClass::SyncOff;
        }

        RadioTransmitter<PIN_CS, PIN_GDO0, PIN_GDO2>::update_state(cls);
    }

public:
    RadioTransmitter(const RadioTransmitter&) = delete;
    RadioTransmitter& operator=(const RadioTransmitter&) = delete;

    static RadioTransmitter& get_instance()
    {
        static RadioTransmitter instance;
        return instance;
    }

    RadioTransmitterInitResult initialize(FrameBuilder* frame_builder)
    {
        return impl.initialize(frame_builder);
    }

    bool transmit(Command cmd)
    {
        return impl.transmit(cmd);
    }

    bool transmit_bursts(Command cmd, uint8_t repeats)
    {
        return impl.transmit_bursts(cmd, repeats);
    }

    bool start_receiving()
    {
        if (!impl.is_initialized() || receiving) {
            return receiving;
        }

        radio.setOOK(true);
        radio.startReceive();

        write_raw_register(cc1101_reg_iocfg0, 0x0D);
        write_raw_register(cc1101_reg_pktctrl0, 0x32);

        last_edge_time = micros();

        pinMode(PIN_GDO0, INPUT);
        attachInterrupt(
            digitalPinToInterrupt(PIN_GDO0),
            RadioTransmitter<PIN_CS, PIN_GDO0, PIN_GDO2>::handle_edge,
            CHANGE);

        receiving = true;
        return true;
    }

    void stop_receiving()
    {
        if (!receiving) {
            return;
        }

        detachInterrupt(digitalPinToInterrupt(PIN_GDO0));
        receiving = false;
    }
};

}
