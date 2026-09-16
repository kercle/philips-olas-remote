#pragma once

#include <RadioLib.h>

#include <config.h>
#include <olas/bit_sequence.h>
#include <olas/protocol.h>
#include <olas/radio_impl.h>

namespace olas {

template <uint8_t PIN_CS, uint8_t PIN_GDO0, uint8_t PIN_GDO2>
class RadioTransmitter {
    Module radio_module;
    CC1101 radio;
    RadioTransmitterImpl impl;

    bool receiving;
    volatile uint64_t last_edge_time;

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

    static void IRAM_ATTR handle_edge()
    {
        // TODO
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
