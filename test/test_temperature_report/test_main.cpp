#include <unity.h>
#include "TemperatureReport.h"

void setUp() {}
void tearDown() {}

void test_default_constructor_is_not_valid()
{
    TemperatureReport report;
    TEST_ASSERT_EQUAL_FLOAT(0.0f, report.GetTemp());
    TEST_ASSERT_EQUAL(0, report.GetTimestamp());
    TEST_ASSERT_EQUAL(false, report.IsValid());
}

void test_parameterized_constructor_stores_temp()
{
    TemperatureReport report(1000, 72.5f);
    TEST_ASSERT_EQUAL_FLOAT(72.5f, report.GetTemp());
}

void test_parameterized_constructor_stores_time()
{
    TemperatureReport report(1000, 72.5f);
    TEST_ASSERT_EQUAL(1000, report.GetTimestamp());
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_default_constructor_is_not_valid);
    RUN_TEST(test_parameterized_constructor_stores_temp);
    RUN_TEST(test_parameterized_constructor_stores_time);
    return UNITY_END();
}
