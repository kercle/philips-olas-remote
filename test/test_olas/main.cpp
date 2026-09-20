#include "../common.h"

#include <olas/controller.h>

void setUp(void) { }
void tearDown(void) { }

void test_olas_result_success(void);

int main(int argc, char** argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_olas_result_success);
    return UNITY_END();
}
