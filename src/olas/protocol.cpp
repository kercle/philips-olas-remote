#include <olas/protocol.h>

namespace olas {

FrameBuilder::FrameBuilder(uint32_t fan_id, uint8_t init_frame_counter)
    : fan_id(fan_id & 0xffffff) // we can only have 24 bits for the fan ID
    , frame_counter(init_frame_counter)
{
}

uint64_t FrameBuilder::build(Command cmd)
{
    uint8_t cmd_byte = (cmd & 0xFC) | (frame_counter & 0x3);
    uint8_t chk_byte = cmd_byte ^ 0x5B;

    uint64_t frame = ((uint64_t)fan_id << 17)
        | ((uint64_t)cmd_byte << 9)
        | ((uint64_t)chk_byte << 1);
    
    frame_counter++;
    return frame;
}

}
