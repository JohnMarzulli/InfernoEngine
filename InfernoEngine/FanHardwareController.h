#ifndef FAN_HARDWARE_CONTROLLER
#define FAN_HARDWARE_CONTROLLER

class FanHardwareController {
public:
  FanHardwareController(
    unsigned int ioPin) {
    _ioPin = ioPin;
  }

  void SetTarget(
    float targetProportion) {
    _target = targetProportion;
  }

  float GetTarget() {
    return _target;
  }

  void Service(
    float targetProportion) {
  }

private:

  unsigned int _ioPin = 0;
  float _target = 0.0;
};

#endif