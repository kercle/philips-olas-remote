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

Command fan_command_from_speed(uint8_t speed)
{
    switch (speed) {
    case 0:
        return Command::FanOff;
    case 1:
        return Command::FanSpeed1;
    case 2:
        return Command::FanSpeed2;
    case 3:
        return Command::FanSpeed3;
    case 4:
        return Command::FanSpeed4;
    case 5:
        return Command::FanSpeed5;
    default:
        // Maximum speed is 6, so anything above
        // this values is interpreted as this
        // maximal speed.
        return Command::FanSpeed6;
    }
}

void handle_received_data_impl(RadioTransmitter& transmitter, FrameBuilder& frame_builder, FanState& state)
{
    auto frame = transmitter.receive_frame();
    if (!frame.has_value()) {
        return;
    }

    auto val = frame.value();

    if (val.get_fan_id() != frame_builder.get_fan_id()) {
        // Not our device.
        // In the future, we can record these
        // IDs for runtime pairing.
        return;
    }

    // We synchronize our counter to the one of external
    // devices.
    frame_builder.set_frame_counter(val.get_counter() + 1);
    state.update_from_command(val.get_command());
}

}
