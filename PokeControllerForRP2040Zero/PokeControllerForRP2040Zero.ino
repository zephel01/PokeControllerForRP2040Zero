#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_NeoPixel.h>
#include <hardware/watchdog.h>

// v1.4.0 新機能インクルード
#include "Common.h"
#include "Presets.h"
#include "DateChange.h"
#include "HighLevelAPI.h"
#include "JapaneseKeyboard.h"

/**
 * RP2040-Zero Switch Controller
 * v1.4.3: Architecture Refactoring (State Machine, No SetCommand), Protocol Optimization
 * v1.4.2: Code Cleanup (Common.h)
 * v1.4.1: Protocol Unification (16bit)
 * v1.4.0: Preset Commands, Date/Year Change, High-Level API, Japanese Keyboard Support
 * v1.3.5: Support Communication Activity LED (Blinking green)
 * v1.3.4: Stability & reliability enhancements (Watchdog, UART recovery, Error checking)
 * v1.3.3: Integrated USB Keyboard HID (Supports "String, Key, Press, Release)
 * v1.3.2: Stick logic refined (Previous value maintenance & 'end' cmd)
 * v1.3.1: Disabled safety timeout for event-driven PC apps
 * v1.3.0: Debugging (CDC), Recovery, Error LED
 */

// ==========================================
// 0) LED (NeoPixel) 設定
// ==========================================
// PIN_NEOPIXEL は Board Variant で定義済み (16)
Adafruit_NeoPixel neopixel(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// LED状態管理
enum LedState { LED_DISCONNECT, LED_IDLE, LED_ACTIVE, LED_ERROR };
static LedState current_led_state = LED_DISCONNECT;
static uint32_t error_blink_start = 0; // エラー表示開始時刻

// ==========================================
// 1) 定数・タイミング設定 (安定性と保守性のための集約)
// ==========================================
static constexpr uint32_t WATCHDOG_TIMEOUT_MS = 10000;   // 10秒に延長
static constexpr uint32_t COMMAND_TIMEOUT_MS = 250;     // 通信途絶判定
static constexpr bool ENABLE_SAFETY_TIMEOUT = false;    // v1.3.1 - v1.3.2 準拠（必要に応じてtrue）
static constexpr uint32_t ERROR_RECOVERY_MS = 500;      // エラー表示時間
static constexpr uint32_t GAMEPAD_REPORT_INTERVAL_MS = 8;
static constexpr uint32_t USB_INIT_TIMEOUT_MS = 2000;
static constexpr uint32_t KEY_TYPE_DELAY_MS = 20;
static constexpr uint32_t LED_ACTIVE_MS = 50;           // コマンド受信時の点灯時間 (30->50msに微増)

static constexpr int UART_TX_PIN = 0;
static constexpr int UART_RX_PIN = 1;
static constexpr uint32_t UART_BAUD = 115200; 

#define RX_BUFFER_SIZE 256
static char rx_buffer[RX_BUFFER_SIZE];
static int  rx_index = 0;
static uint32_t last_command_ms = 0;

// ==========================================
// 2) HID レポート設定 (Gamepad & Keyboard)
// ==========================================

// Switch Gamepad (HORIPAD) Descriptor
uint8_t const gamepad_report_desc[] = {
  0x05, 0x01, 0x09, 0x05, 0xA1, 0x01, 0x15, 0x00, 0x25, 0x01, 0x35, 0x00, 0x45, 0x01, 0x75, 0x01,
  0x95, 0x10, 0x05, 0x09, 0x19, 0x01, 0x29, 0x10, 0x81, 0x02, 0x05, 0x01, 0x25, 0x07, 0x46, 0x3B,
  0x01, 0x75, 0x04, 0x95, 0x01, 0x65, 0x14, 0x09, 0x39, 0x81, 0x42, 0x65, 0x00, 0x95, 0x01, 0x81,
  0x01, 0x26, 0xFF, 0x00, 0x46, 0xFF, 0x00, 0x09, 0x30, 0x09, 0x31, 0x09, 0x32, 0x09, 0x35, 0x75,
  0x08, 0x95, 0x04, 0x81, 0x02, 0x06, 0x00, 0xFF, 0x09, 0x20, 0x95, 0x01, 0x81, 0x02, 0xC0
};

// Standard Keyboard Descriptor
uint8_t const keyboard_report_desc[] = {
  TUD_HID_REPORT_DESC_KEYBOARD()
};

Adafruit_USBD_HID usb_gamepad;
Adafruit_USBD_HID usb_keyboard;

// gp_reportの定義（Common.hで宣言、ここで定義・初期化）
switch_report_t gp_report = {0, HAT_CENTER, STICK_CENTER, STICK_CENTER, STICK_CENTER, STICK_CENTER, 0x00};

// 前方宣言
static void parse_protocol_line(char* line);
static void update_led();
static bool is_hex_char(char c);
static uint8_t ascii_to_hid(char c);

// v1.4.0: gp_reportは非staticとなり、他の.cppファイルからextern参照可能

// リカバリ判定用
static bool was_mounted = false;

// Gamepadレポートの初期化
static void reset_gamepad_report() {
  gp_report.buttons = 0;
  gp_report.hat = HAT_CENTER;
  gp_report.lx = STICK_CENTER;
  gp_report.ly = STICK_CENTER;
  gp_report.rx = STICK_CENTER;
  gp_report.ry = STICK_CENTER;
  gp_report.vendor = 0x00;
}

// ==========================================
// 3) メインロジック
// ==========================================

void setup() {
  watchdog_enable(WATCHDOG_TIMEOUT_MS, 1);

  // USB CDC (デバッグ用シリアル) 開始
  Serial.begin(115200);

  // USB Device 設定
  TinyUSBDevice.detach(); 
  TinyUSBDevice.setID(0x0F0D, 0x00C1);
  TinyUSBDevice.setManufacturerDescriptor("HORI");
  TinyUSBDevice.setProductDescriptor("HORIPAD");

  // Gamepad インスタンス開始
  usb_gamepad.setReportDescriptor(gamepad_report_desc, sizeof(gamepad_report_desc));
  usb_gamepad.setPollInterval(1);
  usb_gamepad.begin();

  // Keyboard インスタンス開始
  usb_keyboard.setReportDescriptor(keyboard_report_desc, sizeof(keyboard_report_desc));
  usb_keyboard.setPollInterval(1);
  usb_keyboard.begin();

  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin();
  }

  // USB初期化完了待ち (1s以上の固定待ちを避け、最大2s待機)
  uint32_t usb_init_start = millis();
  while (!TinyUSBDevice.isInitialized() && millis() - usb_init_start < USB_INIT_TIMEOUT_MS) {
    watchdog_update();
    delay(10);
  }
  TinyUSBDevice.attach();
  
  // LED 初期化
  neopixel.begin();
  neopixel.setBrightness(30); 
  update_led();

  // UART (Poke-Controller 通信用) 初期化
  Serial1.setTX(UART_TX_PIN);
  Serial1.setRX(UART_RX_PIN);
  Serial1.begin(UART_BAUD);
}

void loop() {
  watchdog_update();

  bool is_mounted = TinyUSBDevice.mounted();
  if (was_mounted && !is_mounted) {
    gp_report.buttons = 0;
    gp_report.hat = HAT_CENTER;
    current_led_state = LED_DISCONNECT;
  } else if (!was_mounted && is_mounted) {
    current_led_state = LED_IDLE;
  }
  was_mounted = is_mounted;

  // 1. UART & USB 受信処理
  // ポートをまたいでパケットが分割されることはない前提で、
  // どちらかデータがある方を優先してバッファに取り込む簡易実装
  Stream* active_serial = nullptr;
  
  if (Serial.available()) {
    active_serial = &Serial;
  } else if (Serial1.available()) {
    active_serial = &Serial1;
  }

  if (active_serial) {
    while (active_serial->available() > 0) {
      char c = (char)active_serial->read();
      
      if (c == '\n' || c == '\r') {
        if (rx_index > 0) {
          rx_buffer[rx_index] = '\0';
          parse_protocol_line(rx_buffer);
          rx_index = 0; 
          last_command_ms = millis();
          current_led_state = LED_ACTIVE;
        }
      } else {
        if (rx_index < RX_BUFFER_SIZE - 1) {
          rx_buffer[rx_index++] = c;
        } else {
          Serial.println("Error: RX Buffer Overflow!");
          rx_index = 0;
          current_led_state = LED_ERROR;
          error_blink_start = millis();
          // 改行まで読み飛ばして同期を戻す
          while (active_serial->available() > 0) {
            if (active_serial->read() == '\n') break;
          }
        }
      }
    }
  }

  if (current_led_state == LED_ERROR) {
    if (millis() - error_blink_start > ERROR_RECOVERY_MS) {
      current_led_state = is_mounted ? LED_IDLE : LED_DISCONNECT;
    }
  } 
  else if (current_led_state == LED_ACTIVE && (millis() - last_command_ms > LED_ACTIVE_MS)) {
    // 通信LEDを一定時間で戻す (activity blink)
    current_led_state = is_mounted ? LED_IDLE : LED_DISCONNECT;
  }
  else if (ENABLE_SAFETY_TIMEOUT && (millis() - last_command_ms > COMMAND_TIMEOUT_MS)) {
    reset_gamepad_report();
    usb_keyboard.keyboardRelease(0);
    current_led_state = LED_IDLE;
  }

  update_led();

  // v1.4.0: プリセット状態更新
  update_preset_state();

  // Gamepad Report 送信
  static uint32_t last_report_ms = 0;
  uint32_t now = millis();
  if (now - last_report_ms >= GAMEPAD_REPORT_INTERVAL_MS) {
    last_report_ms = now;
    if (is_mounted && usb_gamepad.ready()) {
      usb_gamepad.sendReport(0, &gp_report, sizeof(gp_report));
    }
  }
}

// LED更新関数 (非ブロッキング)
static void update_led() {
  uint32_t color = 0;
  switch (current_led_state) {
    case LED_DISCONNECT: color = neopixel.Color(50, 0, 0); break;
    case LED_IDLE:       color = neopixel.Color(0, 0, 10); break; // 待機中は暗い青 (50 -> 10)
    case LED_ACTIVE:     color = neopixel.Color(0, 100, 0); break; // 通信中は明るい緑 (50 -> 100)
    case LED_ERROR:
      if ((millis() / 100) % 2 == 0) color = neopixel.Color(100, 0, 0);
      else color = 0;
      break;
  }
  neopixel.setPixelColor(0, color);
  neopixel.show();
}

static bool is_hex_char(char c) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

// ASCII -> HID Keycode (簡易版)
static uint8_t ascii_to_hid(char c) {
  if (c >= 'a' && c <= 'z') return HID_KEY_A + (c - 'a');
  if (c >= 'A' && c <= 'Z') return HID_KEY_A + (c - 'A');
  if (c >= '1' && c <= '9') return HID_KEY_1 + (c - '1');
  if (c == '0') return HID_KEY_0;
  if (c == ' ') return HID_KEY_SPACE;
  if (c == '-') return HID_KEY_MINUS;
  if (c == '.') return HID_KEY_PERIOD;
  return 0;
}

// プロトコル解析関数
static void parse_protocol_line(char* line) {
  if (strlen(line) < 1) return;

  // ==================== v1.4.0: プリセットコマンド ====================
  if (!is_hex_char(line[0])) {
    // プリセットコマンド
    parse_preset_command(line);

    // 日付・年変更コマンド
    DateChangeCommand date_cmd;
    int years;
    if (parse_date_command(line, &date_cmd)) {
      execute_date_change(&date_cmd);
      return;
    }
    if (parse_year_command(line, &years)) {
      execute_year_change(years);
      return;
    }

    // 既存の特殊コマンドに該当しない場合はreturnしない
    // 既存の"end"などの処理に続く
  }

  // 1. 文字列タイピング（v1.4.0: JIS対応版に更新）
  if (line[0] == '"') {
    Serial.printf("Keyboard: Typing JP string [%s]\n", &line[1]);
    type_jp_string(&line[1]);
    return;
  }

  // 2. 個別キー操作: Key/Press/Release (Raw HID Keycode)
  if (strncmp(line, "Key ", 4) == 0) {
    char* endptr;
    uint8_t k = (uint8_t)strtoul(&line[4], &endptr, 16);
    if (endptr != &line[4]) {
      uint8_t keys[6] = { k, 0, 0, 0, 0, 0 };
      usb_keyboard.keyboardReport(0, 0, keys);
      delay(KEY_TYPE_DELAY_MS);
      usb_keyboard.keyboardRelease(0);
    }
    return;
  }
  if (strncmp(line, "Press ", 6) == 0) {
    char* endptr;
    uint8_t k = (uint8_t)strtoul(&line[6], &endptr, 16);
    if (endptr != &line[6]) {
      uint8_t keys[6] = { k, 0, 0, 0, 0, 0 };
      usb_keyboard.keyboardReport(0, 0, keys);
    }
    return;
  }
  if (strncmp(line, "Release ", 8) == 0) {
    // 特定キーのReleaseは管理が複雑なため、現在はRelease Allのみとする
    // もし特定キーだけ離したい場合は現在のReport状態を管理する必要がある
    usb_keyboard.keyboardRelease(0); 
    return;
  }

  // 3. 'end' コマンド: 全てをニュートラルに戻す
  if (strncmp(line, "end", 3) == 0) {
    reset_gamepad_report();
    usb_keyboard.keyboardRelease(0);
    Serial.println("Command: end (Reset all)");
    return;
  }

  // 4. 標準 Gamepad プロトコル (HEX)
  if (!is_hex_char(line[0])) return;

  char* p = line;
  uint16_t raw_btns = (uint16_t)strtoul(p, &p, 16);
  while (*p == ' ') p++;
  uint8_t hat = (uint8_t)strtoul(p, &p, 16);
  while (*p == ' ') p++;
  uint8_t lx = (uint8_t)strtoul(p, &p, 16);
  while (*p == ' ') p++;
  uint8_t ly = (uint8_t)strtoul(p, &p, 16);
  while (*p == ' ') p++;
  uint8_t rx = (uint8_t)strtoul(p, &p, 16);
  while (*p == ' ') p++;
  uint8_t ry = (uint8_t)strtoul(p, &p, 16);

  bool use_right = raw_btns & 0x01;
  bool use_left  = raw_btns & 0x02;
  gp_report.buttons = raw_btns >> 2;
  gp_report.hat = hat;

  if (use_left && use_right) {
    gp_report.lx = lx; gp_report.ly = ly;
    gp_report.rx = rx; gp_report.ry = ry;
  } else if (use_right) {
    gp_report.rx = lx; gp_report.ry = ly;
  } else if (use_left) {
    gp_report.lx = lx; gp_report.ly = ly;
  }
}
