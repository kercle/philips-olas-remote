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

    RadioTransmitter()
        : radio_module(PIN_CS, PIN_GDO0, RADIOLIB_NC, PIN_GDO2)
        , radio(&radio_module)
        , impl(radio_module, radio)
    {
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
};

}
