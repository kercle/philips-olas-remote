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

Controller::Controller(uint32_t fan_id, uint8_t bursts)
    : frame_builder(fan_id, 0)
    , transmitter(config::pin_cs, config::pin_gdo0, config::pin_gdo2)
    , bursts(bursts)
{
}

RadioTransmitterResult Controller::initialize()
{
    return transmitter.initialize(&frame_builder);
}

void Controller::handle_received_data()
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

    switch (val.get_command()) {
    case Command::FanOff:
        state.fan = 0;
        break;
    case Command::FanSpeed1:
        state.fan = 1;
        break;
    case Command::FanSpeed2:
        state.fan = 2;
        break;
    case Command::FanSpeed3:
        state.fan = 3;
        break;
    case Command::FanSpeed4:
        state.fan = 4;
        break;
    case Command::FanSpeed5:
        state.fan = 5;
        break;
    case Command::FanSpeed6:
        state.fan = 6;
        break;
    case Command::ReverseDirection:
        state.reversed = !state.reversed;
        break;
    case Command::LightOff:
        state.light = false;
        break;
    case Command::LightOn:
        state.light = true;
        break;
    }
}

FanState Controller::get_state() const
{
    return state;
}

void Controller::light_on()
{
    transmitter.transmit_bursts(Command::LightOn, bursts);
}

void Controller::light_off()
{
    transmitter.transmit_bursts(Command::LightOff, bursts);
}

void Controller::fan_on(uint8_t speed)
{
    switch (speed) {
    case 0:
        fan_off();
        break;
    case 1:
        transmitter.transmit_bursts(Command::FanSpeed1, bursts);
        break;
    case 2:
        transmitter.transmit_bursts(Command::FanSpeed2, bursts);
        break;
    case 3:
        transmitter.transmit_bursts(Command::FanSpeed3, bursts);
        break;
    case 4:
        transmitter.transmit_bursts(Command::FanSpeed4, bursts);
        break;
    case 5:
        transmitter.transmit_bursts(Command::FanSpeed5, bursts);
        break;
    default:
        // Maximum speed is 6, so anything above
        // this values is interpreted as this
        // maximal speed.
        transmitter.transmit_bursts(Command::FanSpeed6, bursts);
    }
}

void Controller::fan_off()
{
    transmitter.transmit_bursts(Command::FanOff, bursts);
}

void Controller::increase_brightness(uint8_t steps)
{
    for (uint8_t i = 0; i < steps; ++i) {
        transmitter.transmit_bursts(Command::LightBrightnessUp, bursts);
    }
}

void Controller::decrease_brightness(uint8_t steps)
{
    for (uint8_t i = 0; i < steps; ++i) {
        transmitter.transmit_bursts(Command::LightBrightnessDown, bursts);
    }
}

void Controller::sleep_wind()
{
    transmitter.transmit_bursts(Command::SleepWind, bursts);
}

void Controller::reverse_direction()
{
    transmitter.transmit_bursts(Command::ReverseDirection, bursts);
}

}
