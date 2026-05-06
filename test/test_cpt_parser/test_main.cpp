#include <unity.h>
#include "CombustionBleParser.h"

// Live capture: CPT serial 268497964 at room temperature (~23°C).
// Payload: 01 2c f4 00 10 54 a3 6a 54 8d aa 51 35 a8 06 d5 f0 1a 00 e0 00 00
static const uint8_t CPT_PAYLOAD[] = {
    0x01, 0x2c, 0xf4, 0x00, 0x10, 0x54, 0xa3, 0x6a,
    0x54, 0x8d, 0xaa, 0x51, 0x35, 0xa8, 0x06, 0xd5,
    0xf0, 0x1a, 0x00, 0xe0, 0x00, 0x00
};
static const uint8_t CPT_PAYLOAD_LEN = sizeof(CPT_PAYLOAD);

// Tolerance: half the CPT resolution (0.05°C / 2)
static const float TEMP_DELTA = 0.025f;

void setUp() {}
void tearDown() {}

void test_cpt_parse_is_valid()
{
    CombustionAdvertisement ad = CombustionBleParser::Parse(CPT_PAYLOAD, CPT_PAYLOAD_LEN);
    TEST_ASSERT_TRUE(ad.isValid);
}

void test_cpt_product_type()
{
    CombustionAdvertisement ad = CombustionBleParser::Parse(CPT_PAYLOAD, CPT_PAYLOAD_LEN);
    TEST_ASSERT_EQUAL_UINT8(0x01, ad.productType);
}

void test_cpt_serial_number()
{
    CombustionAdvertisement ad = CombustionBleParser::Parse(CPT_PAYLOAD, CPT_PAYLOAD_LEN);
    TEST_ASSERT_EQUAL_UINT32(268497964UL, ad.serialNumber);
}

void test_cpt_t1_tip()
{
    CombustionAdvertisement ad = CombustionBleParser::Parse(CPT_PAYLOAD, CPT_PAYLOAD_LEN);
    TEST_ASSERT_FLOAT_WITHIN(TEMP_DELTA, 23.10f, ad.temperatures[0]);
}

void test_cpt_t2()
{
    CombustionAdvertisement ad = CombustionBleParser::Parse(CPT_PAYLOAD, CPT_PAYLOAD_LEN);
    TEST_ASSERT_FLOAT_WITHIN(TEMP_DELTA, 22.60f, ad.temperatures[1]);
}

void test_cpt_t3()
{
    CombustionAdvertisement ad = CombustionBleParser::Parse(CPT_PAYLOAD, CPT_PAYLOAD_LEN);
    TEST_ASSERT_FLOAT_WITHIN(TEMP_DELTA, 22.60f, ad.temperatures[2]);
}

void test_cpt_t4()
{
    CombustionAdvertisement ad = CombustionBleParser::Parse(CPT_PAYLOAD, CPT_PAYLOAD_LEN);
    TEST_ASSERT_FLOAT_WITHIN(TEMP_DELTA, 22.65f, ad.temperatures[3]);
}

void test_cpt_t5()
{
    CombustionAdvertisement ad = CombustionBleParser::Parse(CPT_PAYLOAD, CPT_PAYLOAD_LEN);
    TEST_ASSERT_FLOAT_WITHIN(TEMP_DELTA, 22.65f, ad.temperatures[4]);
}

void test_cpt_t6()
{
    CombustionAdvertisement ad = CombustionBleParser::Parse(CPT_PAYLOAD, CPT_PAYLOAD_LEN);
    TEST_ASSERT_FLOAT_WITHIN(TEMP_DELTA, 22.65f, ad.temperatures[5]);
}

void test_cpt_t7()
{
    CombustionAdvertisement ad = CombustionBleParser::Parse(CPT_PAYLOAD, CPT_PAYLOAD_LEN);
    TEST_ASSERT_FLOAT_WITHIN(TEMP_DELTA, 22.65f, ad.temperatures[6]);
}

void test_cpt_t8_ambient()
{
    CombustionAdvertisement ad = CombustionBleParser::Parse(CPT_PAYLOAD, CPT_PAYLOAD_LEN);
    TEST_ASSERT_FLOAT_WITHIN(TEMP_DELTA, 22.60f, ad.temperatures[7]);
    TEST_ASSERT_FLOAT_WITHIN(TEMP_DELTA, 22.60f, ad.GetAmbientTemp());
}

void test_cpt_too_short_is_invalid()
{
    CombustionAdvertisement ad = CombustionBleParser::Parse(CPT_PAYLOAD, 10);
    TEST_ASSERT_FALSE(ad.isValid);
}

void test_cpt_wrong_product_type_is_invalid()
{
    uint8_t bad[CPT_PAYLOAD_LEN];
    memcpy(bad, CPT_PAYLOAD, CPT_PAYLOAD_LEN);
    bad[0] = 0x03;  // GGG type
    CombustionAdvertisement ad = CombustionBleParser::Parse(bad, CPT_PAYLOAD_LEN);
    TEST_ASSERT_FALSE(ad.isValid);
}

void test_cpt_all_zero_temps_is_invalid()
{
    uint8_t zeroed[CPT_PAYLOAD_LEN];
    memcpy(zeroed, CPT_PAYLOAD, CPT_PAYLOAD_LEN);
    for (int i = 5; i < 18; i++) zeroed[i] = 0x00;
    CombustionAdvertisement ad = CombustionBleParser::Parse(zeroed, CPT_PAYLOAD_LEN);
    TEST_ASSERT_FALSE(ad.isValid);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_cpt_parse_is_valid);
    RUN_TEST(test_cpt_product_type);
    RUN_TEST(test_cpt_serial_number);
    RUN_TEST(test_cpt_t1_tip);
    RUN_TEST(test_cpt_t2);
    RUN_TEST(test_cpt_t3);
    RUN_TEST(test_cpt_t4);
    RUN_TEST(test_cpt_t5);
    RUN_TEST(test_cpt_t6);
    RUN_TEST(test_cpt_t7);
    RUN_TEST(test_cpt_t8_ambient);
    RUN_TEST(test_cpt_too_short_is_invalid);
    RUN_TEST(test_cpt_wrong_product_type_is_invalid);
    RUN_TEST(test_cpt_all_zero_temps_is_invalid);
    return UNITY_END();
}
