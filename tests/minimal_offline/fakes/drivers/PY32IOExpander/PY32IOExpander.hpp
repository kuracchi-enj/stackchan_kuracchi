#pragma once

#include <M5StackChan.h>
#include <array>

#define PY32IOEXPANDER_STATUS_API 1

namespace hardware_stub {
inline bool readSucceeds = true;
inline uint8_t gpioOutputLow = 0;
inline std::array<uint8_t, 3> ledColor{};
}

namespace m5 {
class PY32IOExpander_Class {
 public:
  enum gpio_pull_t { pull_none = 0, pull_up = 1, pull_down = 2 };

  bool begin() { return true; }
  bool digitalWrite(uint8_t, bool) { return true; }
  bool setPullMode(uint8_t, gpio_pull_t) { return true; }
  bool setDirection(uint8_t, bool) { return true; }

  // M5Unified 0.2.21＋BSP 1.1.1の値だけ返すAPIは、失敗もLOWになる。
  bool getWriteValue(uint8_t pin) const {
    const uint8_t value = hardware_stub::readSucceeds
                              ? hardware_stub::gpioOutputLow : uint8_t(0);
    return (value & (1U << pin)) != 0;
  }

  bool readRegister(uint8_t reg, uint8_t* result, std::size_t length) const {
    if (reg != 0x05 || length != 1 || !hardware_stub::readSucceeds) {
      return false;
    }
    *result = hardware_stub::gpioOutputLow;
    return true;
  }

  void setDriveMode(uint8_t, bool) {}
  void setLedCount(uint8_t) {}
  void setLedColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index == 0) {
      hardware_stub::ledColor = {r, g, b};
    }
  }
  void refreshLeds() {}
};
}  // namespace m5
