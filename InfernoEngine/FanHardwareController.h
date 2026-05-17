#ifndef FAN_HARDWARE_CONTROLLER
#define FAN_HARDWARE_CONTROLLER

class FanHardwareController {
public:
  FanHardwareController(unsigned int ioPin) {
    _ioPin = ioPin;
    pinMode(_ioPin, OUTPUT);
  }

  void SetTarget(float targetProportion) {
    if (targetProportion < 0.0f) {
      Serial.println("Warning: Clamping _target to 0.");

      targetProportion = 0.0f;
    } else if (targetProportion > 1.0f) {
      Serial.println("Warning: Clamping _target to 1.");

      targetProportion = 1.0f;
    }

    if (_target != targetProportion) {
      Serial.printf("Fan target changing from %f to %f\n", _target,
                    targetProportion);
    }

    _target = targetProportion;
  }

  float GetTarget() { return _target; }

  void Service() { analogWrite(_ioPin, (int)(_target * 255)); }

private:
  unsigned int _ioPin = 0;
  float _target = 0.0;
};

#endif