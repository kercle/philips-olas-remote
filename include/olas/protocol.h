#pragma once

#include <stdint.h>

namespace olas {

enum Command {
    FanOff = 0x10,
    FanSpeed1 = 0x40,
    FanSpeed2 = 0xAC,
    FanSpeed3 = 0x9C,
    FanSpeed4 = 0x20,
    FanSpeed5 = 0x80,
    FanSpeed6 = 0x8C,
    FanForwardReverseToggle = 0x50,
    FanSleep = 0x30,
    FanTimer1h = 0x1,
    FanTimer3h = 0x1,
    FanTimer6h = 0x1,
    LightBrightnessUp = 0x70,
    LightBrightnessDown = 0x28,
    LightWarmWhite = 0x6C,
    LightDayWhite = 0x84,
    LightOn = 0x7C,
    LightOff = 0xBC,
};

class FrameBuilder {
    uint32_t fan_id;
    uint8_t frame_counter;

public:
    FrameBuilder(uint32_t fan_id, uint8_t init_frame_counter);

    uint64_t build(Command cmd);
};

}
