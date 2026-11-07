#pragma once

#include <cstddef>
#include <cstdint>

namespace hardware_stub {
inline uint32_t clockMs = 0;
}

inline uint32_t millis() { return hardware_stub::clockMs; }
inline void delay(uint32_t ms) { hardware_stub::clockMs += ms; }

constexpr int TFT_BLACK = 0;
constexpr int TFT_WHITE = 1;
constexpr int TFT_RED = 2;
constexpr int TFT_GREEN = 3;

// 画面・シリアルは、このI2C異常系の検証対象外。
struct DisplayStub {
  void setBrightness(int) {}
  void fillScreen(int) {}
  void setTextColor(int, int) {}
  void setTextSize(int) {}
  void setCursor(int, int) {}
  void println(const char*) {}
};

struct M5Stub {
  struct Config {
    bool internal_mic = true;
    bool internal_spk = true;
    bool internal_imu = true;
    bool internal_rtc = true;
  };
  DisplayStub Display;
  Config config() { return {}; }
  void begin(Config) {}
  void update() {}
};

struct SerialStub {
  void begin(int) {}
  void println(const char*) {}
};

inline M5Stub M5;
inline SerialStub Serial;
