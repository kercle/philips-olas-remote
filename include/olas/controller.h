#pragma once

#include <olas/protocol.h>
#include <olas/radio.h>

namespace olas {

struct FanState {
    bool light;
    uint8_t fan;
    bool reversed;

    FanState();

    bool operator ==(const FanState& other);
    bool operator !=(const FanState& other);
};

class Controller {
    FrameBuilder frame_builder;
    RadioTransmitter transmitter;
    FanState state;

    uint8_t bursts;

public:
    Controller(uint32_t fan_id, uint8_t bursts = 4);
    RadioTransmitterResult initialize();

    void handle_received_data();
    FanState get_state() const;

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
