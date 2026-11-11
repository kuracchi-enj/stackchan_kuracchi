#include <iostream>
#include <drivers/Si12T/Si12T.h>

#include "../../firmware/arduino/minimal_offline/minimal_offline.ino"

namespace {
void runFor(uint32_t duration, uint8_t touchResult, bool readSucceeds = true) {
  hardware_stub::touchResult = touchResult;
  hardware_stub::touchReadSucceeds = readSucceeds;
  const uint32_t end = millis() + duration;
  while (millis() < end) {
    loop();
  }
}
}  // namespace

int main() {
  const std::array<uint8_t, 3> off{0, 0, 0};
  const std::array<uint8_t, 3> blue{0, 0, 16};
  const std::array<uint8_t, 3> green{0, 16, 0};
  int failures = 0;
  auto expectLed = [&](const char* name, const std::array<uint8_t, 3>& color) {
    if (hardware_stub::ledColor != color) {
      std::cerr << "FAIL: " << name << '\n';
      ++failures;
    } else {
      std::cout << "PASS: " << name << '\n';
    }
  };

  hardware_stub::touchResult = 0x01;
  setup();
  runFor(600, 0x01);
  expectLed("starts blue even if a finger is on the sensor at boot", blue);

  runFor(60, 0x00);
  runFor(60, 0x04);
  expectLed("a new touch changes the lit LEDs to green", green);

  runFor(850, 0x20);
  expectLed("a held touch across zones does not toggle again", green);
  runFor(60, 0x00, false);
  runFor(60, 0x20);
  expectLed("a read failure does not rearm a held touch", green);

  runFor(60, 0x00);
  runFor(10, 0x04);
  runFor(60, 0x00);
  expectLed("a single noisy sample does not change color", green);
  runFor(60, 0x01);
  expectLed("a second touch after release changes green back to blue", blue);

  runFor(250, 0x01);
  expectLed("LEDs still turn off during the blink cycle", off);
  runFor(60, 0x00);
  runFor(60, 0x20);
  expectLed("a touch during the off interval keeps the LEDs dark", off);
  runFor(350, 0x20);
  expectLed("the next blink uses the newly selected color", green);
  return failures ? 1 : 0;
}
