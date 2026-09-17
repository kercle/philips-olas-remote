#pragma once

#include <stdint.h>
#include <optional>

namespace olas {

enum Command : uint8_t {
    FanOff = 0x10,
    FanSpeed1 = 0x40,
    FanSpeed2 = 0xAC,
    FanSpeed3 = 0x9C,
    FanSpeed4 = 0x20,
    FanSpeed5 = 0x80,
    FanSpeed6 = 0x8C,
    ReverseDirection = 0x50,
    SleepWind = 0x30,
    FanTimer1h = 0x1C,
    FanTimer3h = 0x18,
    FanTimer6h = 0x14,
    LightBrightnessUp = 0x70,
    LightBrightnessDown = 0x28,
    LightWarmWhite = 0x6C,
    LightDayWhite = 0x84,
    LightOn = 0x7C,
    LightOff = 0xBC,
};

class Frame {
    friend class FrameBuilder;

    uint64_t data;

    bool is_valid() const;
    Frame(uint64_t data);

public:
    bool get_frame_bit(int8_t msb_first_pos) const;

    static std::optional<Frame> from_data(uint64_t data);

    constexpr uint8_t bit_size()
    {
        return 41;
    }

    uint32_t get_fan_id() const;
    Command get_command() const;
    uint8_t get_counter() const;
};

class FrameBuilder {
    uint32_t fan_id;
    uint8_t frame_counter;

public:
    FrameBuilder(uint32_t fan_id, uint8_t init_frame_counter);

    uint32_t get_fan_id() const;

    Frame build(Command cmd) const;
    void advance_frame_counter();
    void set_frame_counter(uint8_t value);
};

}
