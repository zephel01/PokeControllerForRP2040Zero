#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_NeoPixel.h>
#include <hardware/watchdog.h>

/**
 * RP2040-Zero Switch Controller
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
// 1) UART 設定 & 受信バッファ
// ==========================================
static constexpr int UART_TX_PIN = 0;
static constexpr int UART_RX_PIN = 1;
static constexpr uint32_t UART_BAUD = 115200; 

#define RX_BUFFER_SIZE 256
static char rx_buffer[RX_BUFFER_SIZE];
static int  rx_index = 0;
static uint32_t last_command_ms = 0;

// タイミング微調整 (マイクロ秒)
#define REPORT_DELAY_US 0  

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

typedef struct TU_ATTR_PACKED {
  uint16_t buttons;
  uint8_t  hat;
  uint8_t  lx;
  uint8_t  ly;
  uint8_t  rx;
  uint8_t  ry;
  uint8_t  vendor;
} switch_report_t;

static switch_report_t gp_report = {0, 0x08, 0x80, 0x80, 0x80, 0x80, 0x00};

// 前方宣言
static void parse_protocol_line(char* line);
static void update_led();
static bool is_hex_char(char c);
static uint8_t ascii_to_hid(char c);

// リカバリ判定用
static bool was_mounted = false;

// ==========================================
// 3) メインロジック
// ==========================================

void setup() {
  watchdog_enable(2000, 1);

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

  delay(1000);
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
    gp_report.hat = 0x08;
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
        }
      }
    }
  }

  if (current_led_state == LED_ERROR) {
    if (millis() - error_blink_start > 500) {
      current_led_state = is_mounted ? LED_IDLE : LED_DISCONNECT;
    }
  } 
  // Safety timeout is currently disabled by user (v1.3.1 - v1.3.2)
  /*
  else if (millis() - last_command_ms > 250) { ... }
  */

  update_led();

  // Gamepad Report 送信
  static uint32_t last_ms = 0;
  uint32_t now = millis();
  if (now - last_ms >= 8) {
    last_ms = now;
    if (is_mounted && usb_gamepad.ready()) {
      usb_gamepad.sendReport(0, &gp_report, sizeof(gp_report));
      #if REPORT_DELAY_US > 0
        delayMicroseconds(REPORT_DELAY_US);
      #endif
    }
  }
}

// LED更新関数 (非ブロッキング)
static void update_led() {
  uint32_t color = 0;
  switch (current_led_state) {
    case LED_DISCONNECT: color = neopixel.Color(50, 0, 0); break;
    case LED_IDLE:       color = neopixel.Color(0, 0, 50); break;
    case LED_ACTIVE:     color = neopixel.Color(0, 50, 0); break;
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
  
  // 1. 文字列タイピング: "ABCDE
  if (line[0] == '"') {
    Serial.printf("Keyboard: Typing string [%s]\n", &line[1]);
    for (int i = 1; line[i] != '\0'; i++) {
        // ライブラリの keyboardPress は ASCII文字を受け取る仕様のため、そのまま渡す
        if (usb_keyboard.keyboardPress(0, line[i])) {
            delay(20);
            usb_keyboard.keyboardRelease(0);
            delay(20);
        }
    }
    return;
  }

  // 2. 個別キー操作: Key/Press/Release (Raw HID Keycode)
  if (strncmp(line, "Key ", 4) == 0) {
    uint8_t k = (uint8_t)strtoul(&line[4], NULL, 16);
    uint8_t keys[6] = { k, 0, 0, 0, 0, 0 };
    usb_keyboard.keyboardReport(0, 0, keys); // Start Press
    delay(20);
    usb_keyboard.keyboardRelease(0);         // Release
    return;
  }
  if (strncmp(line, "Press ", 6) == 0) {
    uint8_t k = (uint8_t)strtoul(&line[6], NULL, 16);
    uint8_t keys[6] = { k, 0, 0, 0, 0, 0 };
    usb_keyboard.keyboardReport(0, 0, keys);
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
    gp_report.buttons = 0; gp_report.hat = 0x08;
    gp_report.lx = 0x80; gp_report.ly = 0x80;
    gp_report.rx = 0x80; gp_report.ry = 0x80;
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
