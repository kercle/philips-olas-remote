#include <olas/controller.h>

namespace olas {

FanState::FanState()
    : light(false)
    , fan(0)
    , reversed(false)
{
}

bool FanState::operator==(const FanState& other)
{
    return light == other.light && fan == other.fan && reversed == other.reversed;
}

bool FanState::operator!=(const FanState& other)
{
    return !(*this == other);
}

void FanState::update_from_command(Command cmd)
{
    switch (cmd) {
    case Command::FanOff:
        fan = 0;
        break;
    case Command::FanSpeed1:
        fan = 1;
        break;
    case Command::FanSpeed2:
        fan = 2;
        break;
    case Command::FanSpeed3:
        fan = 3;
        break;
    case Command::FanSpeed4:
        fan = 4;
        break;
    case Command::FanSpeed5:
        fan = 5;
        break;
    case Command::FanSpeed6:
        fan = 6;
        break;
    case Command::ReverseDirection:
        reversed = !reversed;
        break;
    case Command::LightOff:
        light = false;
        break;
    case Command::LightOn:
        light = true;
        break;
    default:
        break;
    }
}

void Controller::invoke_cmd(Command cmd)
{
    auto frm = frame_builder.build(cmd);
    transmitter.transmit_bursts(frm, bursts);
    frame_builder.advance_frame_counter();
}

Controller::Controller(RadioTransmitter& transmitter, uint32_t fan_id, uint8_t bursts)
    : frame_builder(fan_id, 0)
    , transmitter(transmitter)
    , bursts(bursts)
{
}

void Controller::handle_received_data()
{
    auto frame = transmitter.receive_frame();
    while (frame.has_value()) {
        auto val = frame.value();

        if (val.get_fan_id() != frame_builder.get_fan_id()) {
            // Not our device.
            // In the future, we can record these
            // IDs for runtime pairing.
            continue;
        }

        // We synchronize our counter to the one of external
        // devices.
        frame_builder.set_frame_counter(val.get_counter() + 1);
        state.update_from_command(val.get_command());

        frame = transmitter.receive_frame();
    }
}

FanState Controller::get_state() const
{
    return state;
}

void Controller::light_on()
{
    invoke_cmd(Command::LightOn);
    state.light = true;
}

void Controller::light_off()
{
    invoke_cmd(Command::LightOff);
    state.light = false;
}

void Controller::fan_on(uint8_t speed)
{
    switch (speed) {
    case 0:
        invoke_cmd(Command::FanOff);
        break;
    case 1:
        invoke_cmd(Command::FanSpeed1);
        break;
    case 2:
        invoke_cmd(Command::FanSpeed2);
        break;
    case 3:
        invoke_cmd(Command::FanSpeed3);
        break;
    case 4:
        invoke_cmd(Command::FanSpeed4);
        break;
    case 5:
        invoke_cmd(Command::FanSpeed5);
        break;
    default:
        // Maximum speed is 6, so anything above
        // this values is interpreted as this
        // maximal speed.
        invoke_cmd(Command::FanSpeed6);
    }

    state.fan = speed;
}

void Controller::fan_off()
{
    invoke_cmd(Command::FanOff);
    state.fan = 0;
}

void Controller::increase_brightness(uint8_t steps)
{
    for (uint8_t i = 0; i < steps; ++i) {
        invoke_cmd(Command::LightBrightnessUp);
    }
}

void Controller::decrease_brightness(uint8_t steps)
{
    for (uint8_t i = 0; i < steps; ++i) {
        invoke_cmd(Command::LightBrightnessDown);
    }
}

void Controller::sleep_wind()
{
    invoke_cmd(Command::SleepWind);
}

void Controller::reverse_direction()
{
    invoke_cmd(Command::ReverseDirection);
    state.reversed = !state.reversed;
}

}
