#include <olas/controller.h>

namespace olas {

Controller::Controller(uint32_t fan_id, uint8_t bursts)
    : frame_builder(fan_id, 0)
    , transmitter(frame_builder)
    , bursts(bursts)
{
}

RadioTransmitterInitResult Controller::initialize()
{
    return transmitter.initialize();
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

void Controller::increase_brightness(uint8_t steps) { }

void Controller::decrease_brightness(uint8_t steps) { }

void Controller::sleep_wind()
{
    transmitter.transmit_bursts(Command::SleepWind, bursts);
}

void Controller::reverse_direction()
{
    transmitter.transmit_bursts(Command::ReverseDirection, bursts);
}

}
