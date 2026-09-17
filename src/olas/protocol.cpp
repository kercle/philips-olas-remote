#include <olas/protocol.h>

constexpr uint8_t check_xor_constant = 0x5B;

namespace olas {
Frame::Frame(uint64_t data)
    : data(data)
{
}

bool Frame::is_valid() const
{
    // LSB must be 0 and anything above bit 41 must be 0 as well
    if ((data & 1) != 0 || (data >> 41) != 0) {
        return false;
    }

    uint8_t chk = static_cast<uint8_t>((data >> 1) & 0xff);
    uint8_t cmd_and_counter = static_cast<uint8_t>((data >> 9) & 0xff);

    if ((cmd_and_counter ^ check_xor_constant) != chk) {
        return false;
    }

    switch (get_command()) {
    case FanOff:
    case FanSpeed1:
    case FanSpeed2:
    case FanSpeed3:
    case FanSpeed4:
    case FanSpeed5:
    case FanSpeed6:
    case ReverseDirection:
    case SleepWind:
    case FanTimer1h:
    case FanTimer3h:
    case FanTimer6h:
    case LightBrightnessUp:
    case LightBrightnessDown:
    case LightWarmWhite:
    case LightDayWhite:
    case LightOn:
    case LightOff:
        return true;
    }

    return false;
}

std::optional<Frame> Frame::from_data(uint64_t data)
{
    Frame ret(data);
    if (ret.is_valid()) {
        return ret;
    }

    return std::nullopt;
}

bool Frame::get_frame_bit(int8_t msb_first_pos) const
{
    if (msb_first_pos > 40 || msb_first_pos < 0) {
        return false;
    }

    return (data >> (40 - msb_first_pos)) & 1;
}

Command Frame::get_command() const
{
    return static_cast<Command>((data >> 9) & 0xfc);
}

FrameBuilder::FrameBuilder(uint32_t fan_id, uint8_t init_frame_counter)
    : fan_id(fan_id & 0xffffff) // we can only have 24 bits for the fan ID
    , frame_counter(init_frame_counter)
{
}

Frame FrameBuilder::build(Command cmd) const
{
    uint8_t cmd_byte = (cmd & 0xfc) | (frame_counter & 0x3);
    uint8_t chk_byte = cmd_byte ^ check_xor_constant;

    uint64_t frame = ((uint64_t)fan_id << 17)
        | ((uint64_t)cmd_byte << 9)
        | ((uint64_t)chk_byte << 1);

    return Frame(frame);
}

void FrameBuilder::advance_frame_counter()
{
    // decrement unsigned frame counter
    frame_counter = (frame_counter + 3) & 0x03;
}

}
