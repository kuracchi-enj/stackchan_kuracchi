#include <iostream>

// 本番スケッチの初期化を実行する。機器との通信だけをスタブへ置き換える。
#include "../../firmware/arduino/minimal_offline/minimal_offline.ino"

int main() {
  struct Case {
    const char* name;
    bool readSucceeds;
    uint8_t gpioOutputLow;
    bool wantReady;
  };
  const Case cases[] = {
      {"I2C read failure must not become ready", false, 0x00, false},
      {"confirmed servo LOW becomes ready", true, 0x00, true},
      {"servo HIGH must not become ready", true, 0x01, false},
      {"other HIGH outputs do not change servo LOW", true, 0xFE, true},
  };

  int failed = 0;
  for (const auto& test : cases) {
    hardware_stub::clockMs = 0;
    hardware_stub::readSucceeds = test.readSucceeds;
    hardware_stub::gpioOutputLow = test.gpioOutputLow;
    setup();
    if (baseReady != test.wantReady) {
      std::cerr << "FAIL: " << test.name << " (ready=" << baseReady << ")\n";
      ++failed;
    } else {
      std::cout << "PASS: " << test.name << '\n';
    }
  }
  return failed ? 1 : 0;
}
