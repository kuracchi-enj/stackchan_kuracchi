#include <M5StackChan.h>
#include <drivers/PY32IOExpander/PY32IOExpander.hpp>
#include <drivers/Si12T/Si12T.h>

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
constexpr uint8_t kLedIntensity = 16;
constexpr uint8_t kHeadTouchMask = 0x3F;  // 上部3区画の接触レベルを表す下位6ビット。
constexpr uint32_t kBlinkIntervalMs = 500;
constexpr uint32_t kTouchDebounceMs = 40;
constexpr uint32_t kBaseInitTimeoutMs = 1500;

m5::PY32IOExpander_Class base;
Si12T headTouch(SI12T_Type_Low, SI12T_Sensitivity_Level_3);
bool baseReady = false;
bool ledOn = false;
bool useGreen = false;
uint32_t lastBlinkMs = 0;
// 起動時にすでに指が触れていても、一度離してから反応する。
bool stableTouch = true;
bool candidateTouch = true;
uint32_t candidateSinceMs = 0;

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
  const uint8_t intensity = enabled ? kLedIntensity : 0;
  for (uint8_t index = 0; index < kLedCount; ++index) {
    base.setLedColor(index, 0, useGreen ? intensity : 0,
                     useGreen ? 0 : intensity);
  }
  base.refreshLeds();
}

bool headWasPressed(uint32_t nowMs) {
  uint8_t result = 0;
  if (!headTouch.readRegister(SI12T_OUTPUT1_ADDR, &result, sizeof(result))) {
    // 通信エラーは指を離したことにせず、判定途中の入力も破棄する。
    candidateTouch = stableTouch;
    return false;
  }

  const bool touching = (result & kHeadTouchMask) != 0;
  if (touching != candidateTouch) {
    candidateTouch = touching;
    candidateSinceMs = nowMs;
  }
  if (candidateTouch == stableTouch || nowMs - candidateSinceMs < kTouchDebounceMs) {
    return false;
  }
  stableTouch = candidateTouch;
  return stableTouch;
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
  M5.Display.println(" Minimal offline v0.2");

  baseReady = initializeBase();
  if (!baseReady) {
    M5.Display.setTextColor(TFT_RED, TFT_BLACK);
    M5.Display.println(" BASE INIT FAILED");
    M5.Display.println(" Power off and check");
    Serial.println("[boot] base_error; motor_power_state_unknown");
    return;
  }

  headTouch.begin();
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.println(" Offline ready");
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.println(" Servo power: OFF cmd");
  M5.Display.println(" Top touch: LED color");
  Serial.println("[boot] minimal_offline; servo_power_off_requested");
  lastBlinkMs = millis();
}

void loop() {
  M5.update();
  const uint32_t nowMs = millis();
  if (baseReady) {
    const bool colorChanged = headWasPressed(nowMs);
    if (colorChanged) {
      useGreen = !useGreen;
    }
    const bool blinkChanged = nowMs - lastBlinkMs >= kBlinkIntervalMs;
    if (blinkChanged) {
      lastBlinkMs = nowMs;
      ledOn = !ledOn;
    }
    if (blinkChanged || (colorChanged && ledOn)) {
      setLed(ledOn);
    }
  }
  delay(10);
}
