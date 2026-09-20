#include "../common.h"

#include <olas/controller.h>

void setUp(void) { }
void tearDown(void) { }

void test_frame_parsing(void)
{
    // Invalid frame (checksum mismatch)
    auto frm = olas::Frame::from_data(0x14976524eaaul);
    TEST_ASSERT(!frm.has_value());

    // Invalid command in frame
    frm = olas::Frame::from_data((0x149765ul << 17) | (0xfful << 9) | ((0xfful ^ 0x5bul) << 1));
    TEST_ASSERT(!frm.has_value());

    // Invalid command in frame
    frm = olas::Frame::from_data((0x149765ul << 17) | (0x14ul << 9) | ((0x14ul ^ 0x5bul) << 1));
    TEST_ASSERT(frm.has_value());

    auto frm_value = frm.value();
    TEST_ASSERT_EQUAL(frm_value.bit_size(), 41);
    TEST_ASSERT_EQUAL(frm_value.get_fan_id(), 0x149765);
    TEST_ASSERT_EQUAL(frm_value.get_command(), olas::Command::FanTimer6h);
    TEST_ASSERT_EQUAL(frm_value.get_counter(), 0);
}

int main(int argc, char** argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_frame_parsing);
    return UNITY_END();
}
