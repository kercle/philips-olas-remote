#pragma once

#include <olas/protocol.h>
#include <olas/radio.h>

namespace olas {

struct FanState {
    bool light;
    uint8_t fan;
    bool reversed;

    FanState();

    bool operator==(const FanState& other);
    bool operator!=(const FanState& other);

    void update_from_command(Command cmd);
};

class Controller {
    FrameBuilder frame_builder;
    RadioTransmitter& transmitter;
    FanState state;

    uint8_t bursts;

    void invoke_cmd(Command cmd);

public:
    Controller(RadioTransmitter& transmitter, uint32_t fan_id, uint8_t bursts = 4);

    void handle_received_data();

    FanState get_state() const;

    void light_on();
    void light_off();

    void increase_brightness(uint8_t steps);
    void decrease_brightness(uint8_t steps);

    void fan_on(uint8_t speed);
    void fan_off();
    void reverse_direction();
    void sleep_wind();
};

}
