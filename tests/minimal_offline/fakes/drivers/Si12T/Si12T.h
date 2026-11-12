#pragma once

#include <M5StackChan.h>

constexpr uint8_t SI12T_OUTPUT1_ADDR = 0x10;
enum SI12T_Type { SI12T_Type_Low };
enum SI12T_Sensitivity_Level { SI12T_Sensitivity_Level_3 = 3 };

namespace hardware_stub {
inline bool touchInitialized = false;
inline bool touchReadSucceeds = true;
inline uint8_t touchResult = 0;
}

class Si12T {
 public:
  Si12T(uint8_t, uint8_t) {}
  void begin() { hardware_stub::touchInitialized = true; }

  bool readRegister(uint8_t reg, uint8_t* result, std::size_t length) const {
    if (reg != SI12T_OUTPUT1_ADDR || length != 1 ||
        !hardware_stub::touchInitialized || !hardware_stub::touchReadSucceeds) {
      return false;
    }
    *result = hardware_stub::touchResult;
    return true;
  }
};
