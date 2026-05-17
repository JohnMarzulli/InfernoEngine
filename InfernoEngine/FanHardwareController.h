#ifndef FAN_HARDWARE_CONTROLLER
#define FAN_HARDWARE_CONTROLLER

class FanHardwareController {
public:
  FanHardwareController(unsigned int ioPin) {
    _ioPin = ioPin;
    pinMode(_ioPin, OUTPUT);
  }

  void SetTarget(float targetProportion) {
    if (_target < 0.0f) {
      _target = 0.0f;
    } else if (_target > 1.0f) {
      _target = 1.0f;
    }

    _target = targetProportion;

    printf("Setting target fan proportion: %f\n", _target);
  }

  float GetTarget() { return _target; }

  void Service(float targetProportion) {
    _target = targetProportion;
    analogWrite(_ioPin, (int)(_target * 255));
  }

private:
  unsigned int _ioPin = 0;
  float _target = 0.0;
};

#endif