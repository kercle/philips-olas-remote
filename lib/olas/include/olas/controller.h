#pragma once

#include <olas/protocol.h>
#include <olas/radio.h>

namespace olas {

// namespace olas_impl {
//     void handle_received_data_impl(
//         RadioTransmitter& transmitter, FrameBuilder& frame_builder, FanState& state);
// }

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

    void invoke_cmd(Command cmd)
    {
        auto frm = frame_builder.build(cmd);
        transmitter.transmit_bursts(frm, bursts);
        frame_builder.advance_frame_counter();
    }

public:
    Controller(RadioTransmitter& transmitter, uint32_t fan_id, uint8_t bursts = 4)
        : frame_builder(fan_id, 0)
        , transmitter(transmitter)
        , bursts(bursts)
    {
    }

    void handle_received_data()
    {
        handle_received_data_impl(transmitter, frame_builder, state);
    }

    FanState get_state() const
    {
        return state;
    }

    void light_on()
    {
        invoke_cmd(Command::LightOn);
    }

    void light_off()
    {
        invoke_cmd(Command::LightOff);
    }

    void fan_on(uint8_t speed)
    {
        invoke_cmd(fan_command_from_speed(speed));
    }

    void fan_off()
    {
        invoke_cmd(Command::FanOff);
    }

    void increase_brightness(uint8_t steps)
    {
        for (uint8_t i = 0; i < steps; ++i) {
            invoke_cmd(Command::LightBrightnessUp);
        }
    }

    void decrease_brightness(uint8_t steps)
    {
        for (uint8_t i = 0; i < steps; ++i) {
            invoke_cmd(Command::LightBrightnessDown);
        }
    }

    void sleep_wind()
    {
        invoke_cmd(Command::SleepWind);
    }

    void reverse_direction()
    {
        invoke_cmd(Command::ReverseDirection);
    }
};

}
