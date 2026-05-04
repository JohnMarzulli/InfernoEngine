#ifndef GRILL_GAUGE
#define GRILL_GAUGE

class GrillGauge {
public:

  GrillGauge() {
  }

  bool IsConnected() {
    return false;
  }

  bool Connect() {
    return false;
  }

  float GetTemp() {
    return 0.0;
  }

  void Service() {}
};

#endif