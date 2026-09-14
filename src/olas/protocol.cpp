#include <olas/protocol.h>

namespace olas {
Frame::Frame(uint64_t data)
    : data(data)
{
}

bool Frame::get_frame_bit(int8_t msb_first_pos) const
{
    if (msb_first_pos > 40 || msb_first_pos < 0) {
        return false;
    }

    return (data >> (40 - msb_first_pos)) & 1;
}

FrameBuilder::FrameBuilder(uint32_t fan_id, uint8_t init_frame_counter)
    : fan_id(fan_id & 0xffffff) // we can only have 24 bits for the fan ID
    , frame_counter(init_frame_counter)
{
}

Frame FrameBuilder::build(Command cmd) const
{
    uint8_t cmd_byte = (cmd & 0xFC) | (frame_counter & 0x3);
    uint8_t chk_byte = cmd_byte ^ 0x5B;

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
