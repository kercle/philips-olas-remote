#pragma once

#include <olas/protocol.h>
#include <olas/radio.h>

namespace olas {

class Controller {
    FrameBuilder frame_builder;
    RadioTransmitter<config::pin_cs, config::pin_gdo0, config::pin_gdo2> &transmitter;

    uint8_t bursts;

public:
    Controller(uint32_t fan_id, uint8_t bursts = 4);
    RadioTransmitterInitResult initialize();

    void light_on();
    void light_off();

    void fan_on(uint8_t speed);
    void fan_off();

    void increase_brightness(uint8_t steps);
    void decrease_brightness(uint8_t steps);

    void sleep_wind();
    void reverse_direction();
};

}
