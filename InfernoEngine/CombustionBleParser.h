#ifndef COMBUSTION_BLE_PARSER_H
#define COMBUSTION_BLE_PARSER_H

#include "CombustionAdvertisement.h"
#include "GaugeAdvertisement.h"
#include <stdint.h>
#include <string.h>

/**
 * @brief Parses raw BLE manufacturer-specific payloads from Combustion Inc
 * devices. The payload begins AFTER the 2-byte company ID (caller filters by
 * COMPANY_ID).
 *
 * CPT payload layout (Predictive Thermometer, product type 0x01):
 *   [0]     product type  (0x01)
 *   [1..4]  serial number (uint32, little-endian)
 *   [5..17] 8 temperatures packed as 13-bit values (104 bits = 13 bytes)
 *             raw encoding: temp_C = (raw * 0.05) - 20.0
 *             packet order: T8 (handle/ambient) first, T1 (tip) last
 *             stored in CombustionAdvertisement reversed: [0]=T1, [7]=T8
 *
 * GGG payload layout (Giant Grill Gauge, product type 0x03):
 *   [0]     product type  (0x03)
 *   [1..10] serial number (10-byte alphanumeric string)
 *   [11..12] raw temperature: lower 13 bits of uint16 LE
 *             raw encoding: temp_C = (raw * 0.1) - 20.0
 *   [13]    gauge status flags (bit 0=sensor present, bit 1=overheating, bit
 *   2=low battery) [14]    reserved [15..18] high-low alarm status [19] gauge
 *   preferences [20..22] reserved
 *
 */
class CombustionBleParser {
public:
  static const uint16_t COMPANY_ID = 0x09C7;
  static const uint8_t PRODUCT_TYPE_PREDICTIVE_THERMOMETER = 0x01;
  static const uint8_t PRODUCT_TYPE_GRILL_GAUGE = 0x03;

  // CPT: 1 type + 4 serial + 13 temps
  static const uint8_t MIN_CPT_PAYLOAD_LENGTH = 18;
  // GGG: 1 type + 10 serial + 2 temp + 1 status + 1 reserved + 4 alarms
  static const uint8_t MIN_GGG_PAYLOAD_LENGTH = 19;

  // CPT parser — returns invalid advertisement for any other product type.
  static CombustionAdvertisement Parse(const uint8_t *data, uint8_t length) {
    CombustionAdvertisement ad;

    if (length < MIN_CPT_PAYLOAD_LENGTH)
      return ad;
    if (data[0] != PRODUCT_TYPE_PREDICTIVE_THERMOMETER)
      return ad;

    ad.productType = data[0];
    ad.serialNumber = (uint32_t)data[1] | ((uint32_t)data[2] << 8) |
                      ((uint32_t)data[3] << 16) | ((uint32_t)data[4] << 24);

    const uint8_t *tempData = &data[5];

    bool allZero = true;
    for (int j = 0; j < 13; j++) {
      if (tempData[j] != 0) {
        allZero = false;
        break;
      }
    }
    if (allZero)
      return ad; // device not yet measuring

    for (int i = 0; i < COMBUSTION_PROBE_COUNT; i++) {
      // Packet stores T8 (handle/ambient) at index 0, T1 (tip) at index 7.
      // Reverse so temperatures[0]=T1 and temperatures[7]=T8.
      uint16_t raw = Extract13Bits(tempData, COMBUSTION_PROBE_COUNT - 1 - i);
      ad.temperatures[i] = (raw * 0.05f) - 20.0f;
    }

    ad.isValid = true;
    return ad;
  }

  // GGG parser — returns invalid advertisement for any other product type.
  static GaugeAdvertisement ParseGauge(const uint8_t *data, uint8_t length) {
    GaugeAdvertisement ad;

    if (length < MIN_GGG_PAYLOAD_LENGTH)
      return ad;
    if (data[0] != PRODUCT_TYPE_GRILL_GAUGE)
      return ad;

    memcpy(ad.serialNumber, &data[1], 10);
    ad.serialNumber[10] = '\0';

    uint16_t raw16 = (uint16_t)data[11] | ((uint16_t)data[12] << 8);
    uint16_t raw13 = raw16 & 0x1FFF;
    ad.temperature = (raw13 * 0.1f) - 20.0f;

    uint8_t flags = data[13];
    ad.sensorPresent = (flags & 0x01) != 0;
    ad.sensorOverheating = (flags & 0x02) != 0;
    ad.lowBattery = (flags & 0x04) != 0;

    // byte [14] = reserved
    ad.highAlarm = DecodeAlarm((uint16_t)data[15] | ((uint16_t)data[16] << 8));
    ad.lowAlarm = DecodeAlarm((uint16_t)data[17] | ((uint16_t)data[18] << 8));

    ad.isValid = true;
    return ad;
  }

private:
  static AlarmStatus DecodeAlarm(uint16_t raw16) {
    AlarmStatus alarm;
    alarm.set = (raw16 & 0x0001) != 0;
    alarm.tripped = (raw16 & 0x0002) != 0;
    alarm.alarming = (raw16 & 0x0004) != 0;
    alarm.threshold = ((raw16 >> 3) * 0.1f) - 20.0f;
    return alarm;
  }

  static uint16_t Extract13Bits(const uint8_t *data, int index) {
    uint32_t startBit = (uint32_t)index * 13;
    uint32_t startByte = startBit / 8;
    uint32_t bitOffset = startBit % 8;

    uint32_t raw =
        (uint32_t)data[startByte] | ((uint32_t)data[startByte + 1] << 8);
    if (bitOffset > 3) {
      raw |= ((uint32_t)data[startByte + 2] << 16);
    }
    return (uint16_t)((raw >> bitOffset) & 0x1FFF);
  }
};

#endif
