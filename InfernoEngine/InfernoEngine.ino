#include "CombustionDeviceManager.h"
#include "FanHardwareController.h"
#include "TemperatureHistory.h"
#include "TemperatureReport.h"
#include "ThermalControlManager.h"
#include "UnconnectedControlManager.h"
#include <BTstackLib.h>

#define FAN_CONTROL_OUTPUT_PIN 0
#define TARGET_PIT_TEMPERATURE_C 120.0f
CombustionDeviceManager _combustionDeviceManager;
FanHardwareController _fanHardwareController(FAN_CONTROL_OUTPUT_PIN);
UnconnectedControlManager _unconnectedControlManger;
ThermalControlManager _thermalControlManager;
unsigned long _startTime;
unsigned long _lastUpdateTime;

// Walk raw BLE AD structures and return a pointer to Manufacturer Specific
// Data (type 0xFF), or nullptr if not present. outLen is set to the number
// of bytes INCLUDING the 2-byte company ID.
static const uint8_t *FindMfgData(const uint8_t *payload, int len,
                                  uint8_t *outLen) {
  int i = 0;
  while (i < len) {
    uint8_t adLen = payload[i];
    if (adLen == 0 || i + adLen >= len)
      break;
    if (payload[i + 1] == 0xFF && adLen >= 3) {
      *outLen = adLen - 1;
      return &payload[i + 2];
    }
    i += 1 + adLen;
  }
  return nullptr;
}

static void OnBleAdvertisement(BLEAdvertisement *adv) {
  uint8_t mfgLen = 0;
  const uint8_t *mfg =
      FindMfgData(adv->getAdvData(), LE_ADVERTISING_DATA_SIZE, &mfgLen);
  if (!mfg || mfgLen < 3)
    return;

  uint16_t companyId = (uint16_t)mfg[0] | ((uint16_t)mfg[1] << 8);
  if (companyId != CombustionBleParser::COMPANY_ID)
    return;

  _combustionDeviceManager.OnManufacturerPayload(&mfg[2], mfgLen - 2);
}

void setup() {
  Serial.begin(115200);

  _startTime = millis();
  _lastUpdateTime = _startTime - 10000;

  // Validate config

  // Create CombustionConnectionManager

  _fanHardwareController.SetTarget(0.0);

  _unconnectedControlManger = UnconnectedControlManager();
  _thermalControlManager = ThermalControlManager();
  _thermalControlManager.SetTargetTemperature(TARGET_PIT_TEMPERATURE_C);

  BTstack.setBLEAdvertisementCallback(OnBleAdvertisement);
  BTstack.setup();
  BTstack.bleStartScanning();
}

void loop() {
  BTstack.loop();

  unsigned long timeStamp = millis();
  unsigned long timeBetweenUpdates = 1000;
  // put your main code here, to run repeatedly:
  unsigned long timeSinceLastUpdate = timeStamp - _lastUpdateTime;

  // Attempt not to flood the serial connection with debug spam
  if (timeSinceLastUpdate < 100) {
    return;
  }

  _combustionDeviceManager.Service();

  bool isConnected = _combustionDeviceManager.IsConnected();

  if (timeSinceLastUpdate > timeBetweenUpdates && isConnected) {
    _lastUpdateTime = millis();

    float currentPitTemperature =
        _combustionDeviceManager.GetCurrentTemperature();
    TemperatureReport newReport =
        TemperatureReport(timeStamp, currentPitTemperature);

    _thermalControlManager.AddReport(newReport);
    Serial.printf("Added report. Temperature: %fC\n", currentPitTemperature);
  }

  float targetFanProportion = 0.0f;

  if (isConnected) {
    _thermalControlManager.Service();
    targetFanProportion = _thermalControlManager.GetTargetFanProportion();
  } else {
    _unconnectedControlManger.Service();
    targetFanProportion = _unconnectedControlManger.GetTargetFanProportion();
  }

  _fanHardwareController.SetTarget(targetFanProportion);
  _fanHardwareController.Service();
}
