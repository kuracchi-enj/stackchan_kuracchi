#include <M5StackChan.h>
#include <drivers/PY32IOExpander/PY32IOExpander.hpp>

// 対象はM5Stack製StackChan完成品。実機の型番確認後に書き込む。
// M5StackChan.begin()はサーボ電源も有効にするため、ここでは使わない。
// Wi-Fi、BLE、マイク、カメラ、音声再生、サーボ位置制御は起動しない。
static_assert(PY32IOEXPANDER_STATUS_API,
              "Use M5StackChan 1.1.1 and M5Unified 0.2.21 or newer.");

namespace {
constexpr uint8_t kServoPowerPin = 0;
constexpr uint8_t kGpioOutputLowRegister = 0x05;
constexpr uint8_t kLedOutputPin = 13;
constexpr uint8_t kLedCount = 12;
constexpr uint8_t kLedBlue = 16;  // 最初のカスタムは、この色を変える。
constexpr uint32_t kBlinkIntervalMs = 500;
constexpr uint32_t kBaseInitTimeoutMs = 1500;

m5::PY32IOExpander_Class base;
bool baseReady = false;
bool ledOn = false;
uint32_t lastBlinkMs = 0;

bool initializeBase() {
  const uint32_t startedMs = millis();
  bool detected = false;
  while (millis() - startedMs < kBaseInitTimeoutMs) {
    if (base.begin()) {
      detected = true;
      break;
    }
    delay(50);
  }
  if (!detected) {
    return false;
  }

  // 出力をLOWにしてから出力モードにする。サーボへ位置命令は送らない。
  if (!base.digitalWrite(kServoPowerPin, false) ||
      !base.setPullMode(kServoPowerPin,
                        m5::PY32IOExpander_Class::pull_down) ||
      !base.setDirection(kServoPowerPin, true)) {
    return false;
  }
  // getWriteValue()は読み取り失敗もfalseにするため、成功状態と値を分ける。
  uint8_t gpioOutputs = 0;
  if (!base.readRegister(kGpioOutputLowRegister, &gpioOutputs,
                         sizeof(gpioOutputs)) ||
      (gpioOutputs & (1U << kServoPowerPin)) != 0) {
    return false;
  }

  if (!base.setPullMode(kLedOutputPin,
                        m5::PY32IOExpander_Class::pull_up) ||
      !base.setDirection(kLedOutputPin, true)) {
    return false;
  }
  base.setDriveMode(kLedOutputPin, false);
  base.setLedCount(kLedCount);
  for (uint8_t index = 0; index < kLedCount; ++index) {
    base.setLedColor(index, uint8_t(0), uint8_t(0), uint8_t(0));
  }
  base.refreshLeds();
  return true;
}

void setLed(bool enabled) {
  for (uint8_t index = 0; index < kLedCount; ++index) {
    base.setLedColor(index, uint8_t(0), uint8_t(0),
                     enabled ? kLedBlue : uint8_t(0));
  }
  base.refreshLeds();
}
}  // namespace

void setup() {
  auto config = M5.config();
  config.internal_mic = false;
  config.internal_spk = false;
  config.internal_imu = false;
  config.internal_rtc = false;
  M5.begin(config);

  Serial.begin(115200);
  M5.Display.setBrightness(64);
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(12, 16);
  M5.Display.println("StackChan");
  M5.Display.println(" Minimal offline v0.1");

  baseReady = initializeBase();
  if (!baseReady) {
    M5.Display.setTextColor(TFT_RED, TFT_BLACK);
    M5.Display.println(" BASE INIT FAILED");
    M5.Display.println(" Power off and check");
    Serial.println("[boot] base_error; motor_power_state_unknown");
    return;
  }

  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.println(" Offline ready");
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.println(" Servo power: OFF cmd");
  M5.Display.println(" LED: dim blue blink");
  Serial.println("[boot] minimal_offline; servo_power_off_requested");
  lastBlinkMs = millis();
}

void loop() {
  M5.update();
  const uint32_t nowMs = millis();
  if (baseReady && nowMs - lastBlinkMs >= kBlinkIntervalMs) {
    lastBlinkMs = nowMs;
    ledOn = !ledOn;
    setLed(ledOn);
  }
  delay(10);
}
