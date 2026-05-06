#include <unity.h>
#include "CombustionBleParser.h"

// Live capture: GGG serial G0000006BC at room temperature (~17°C).
// Payload: 03 47 30 30 30 30 30 30 36 42 43 6e 01 01 64 d9 70 0b 33 00 00 00
static const uint8_t GGG_PAYLOAD[] = {
    0x03, 0x47, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x36, 0x42, 0x43, 0x6e, 0x01, 0x01, 0x64, 0xd9,
    0x70, 0x0b, 0x33, 0x00, 0x00, 0x00
};
static const uint8_t GGG_PAYLOAD_LEN = sizeof(GGG_PAYLOAD);

// Tolerance: half the GGG resolution (0.1°C / 2)
static const float TEMP_DELTA = 0.05f;

void setUp() {}
void tearDown() {}

void test_ggg_parse_is_valid()
{
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(GGG_PAYLOAD, GGG_PAYLOAD_LEN);
    TEST_ASSERT_TRUE(ad.isValid);
}

void test_ggg_serial_number()
{
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(GGG_PAYLOAD, GGG_PAYLOAD_LEN);
    TEST_ASSERT_EQUAL_STRING("G0000006BC", ad.serialNumber);
}

void test_ggg_temperature()
{
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(GGG_PAYLOAD, GGG_PAYLOAD_LEN);
    TEST_ASSERT_FLOAT_WITHIN(TEMP_DELTA, 16.60f, ad.temperature);
}

void test_ggg_sensor_present()
{
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(GGG_PAYLOAD, GGG_PAYLOAD_LEN);
    TEST_ASSERT_TRUE(ad.sensorPresent);
}

void test_ggg_not_overheating()
{
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(GGG_PAYLOAD, GGG_PAYLOAD_LEN);
    TEST_ASSERT_FALSE(ad.sensorOverheating);
}

void test_ggg_not_low_battery()
{
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(GGG_PAYLOAD, GGG_PAYLOAD_LEN);
    TEST_ASSERT_FALSE(ad.lowBattery);
}

void test_ggg_high_alarm_is_set()
{
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(GGG_PAYLOAD, GGG_PAYLOAD_LEN);
    TEST_ASSERT_TRUE(ad.highAlarm.set);
}

void test_ggg_high_alarm_not_tripped()
{
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(GGG_PAYLOAD, GGG_PAYLOAD_LEN);
    TEST_ASSERT_FALSE(ad.highAlarm.tripped);
}

void test_ggg_high_alarm_not_alarming()
{
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(GGG_PAYLOAD, GGG_PAYLOAD_LEN);
    TEST_ASSERT_FALSE(ad.highAlarm.alarming);
}

void test_ggg_high_alarm_threshold()
{
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(GGG_PAYLOAD, GGG_PAYLOAD_LEN);
    TEST_ASSERT_FLOAT_WITHIN(TEMP_DELTA, 341.10f, ad.highAlarm.threshold);
}

void test_ggg_low_alarm_is_set()
{
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(GGG_PAYLOAD, GGG_PAYLOAD_LEN);
    TEST_ASSERT_TRUE(ad.lowAlarm.set);
}

void test_ggg_low_alarm_is_tripped()
{
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(GGG_PAYLOAD, GGG_PAYLOAD_LEN);
    TEST_ASSERT_TRUE(ad.lowAlarm.tripped);
}

void test_ggg_low_alarm_not_alarming()
{
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(GGG_PAYLOAD, GGG_PAYLOAD_LEN);
    TEST_ASSERT_FALSE(ad.lowAlarm.alarming);
}

void test_ggg_low_alarm_threshold()
{
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(GGG_PAYLOAD, GGG_PAYLOAD_LEN);
    TEST_ASSERT_FLOAT_WITHIN(TEMP_DELTA, 143.30f, ad.lowAlarm.threshold);
}

void test_ggg_too_short_is_invalid()
{
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(GGG_PAYLOAD, 10);
    TEST_ASSERT_FALSE(ad.isValid);
}

void test_ggg_wrong_product_type_is_invalid()
{
    uint8_t bad[GGG_PAYLOAD_LEN];
    memcpy(bad, GGG_PAYLOAD, GGG_PAYLOAD_LEN);
    bad[0] = 0x01;  // CPT type
    GaugeAdvertisement ad = CombustionBleParser::ParseGauge(bad, GGG_PAYLOAD_LEN);
    TEST_ASSERT_FALSE(ad.isValid);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_ggg_parse_is_valid);
    RUN_TEST(test_ggg_serial_number);
    RUN_TEST(test_ggg_temperature);
    RUN_TEST(test_ggg_sensor_present);
    RUN_TEST(test_ggg_not_overheating);
    RUN_TEST(test_ggg_not_low_battery);
    RUN_TEST(test_ggg_high_alarm_is_set);
    RUN_TEST(test_ggg_high_alarm_not_tripped);
    RUN_TEST(test_ggg_high_alarm_not_alarming);
    RUN_TEST(test_ggg_high_alarm_threshold);
    RUN_TEST(test_ggg_low_alarm_is_set);
    RUN_TEST(test_ggg_low_alarm_is_tripped);
    RUN_TEST(test_ggg_low_alarm_not_alarming);
    RUN_TEST(test_ggg_low_alarm_threshold);
    RUN_TEST(test_ggg_too_short_is_invalid);
    RUN_TEST(test_ggg_wrong_product_type_is_invalid);
    return UNITY_END();
}
