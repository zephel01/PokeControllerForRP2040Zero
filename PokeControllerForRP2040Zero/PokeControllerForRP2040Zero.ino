#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_NeoPixel.h>
#include <hardware/watchdog.h>

/**
 * RP2040-Zero Switch Controller
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
// 環境によって Switch が入力を取りこぼす場合、この値を増やしてみてください (例: 500 - 4000)
#define REPORT_DELAY_US 0  

// ==========================================
// 2) HID レポート設定
// ==========================================
uint8_t const hid_report_desc[] = {
  0x05, 0x01, 0x09, 0x05, 0xA1, 0x01, 0x15, 0x00, 0x25, 0x01, 0x35, 0x00, 0x45, 0x01, 0x75, 0x01,
  0x95, 0x10, 0x05, 0x09, 0x19, 0x01, 0x29, 0x10, 0x81, 0x02, 0x05, 0x01, 0x25, 0x07, 0x46, 0x3B,
  0x01, 0x75, 0x04, 0x95, 0x01, 0x65, 0x14, 0x09, 0x39, 0x81, 0x42, 0x65, 0x00, 0x95, 0x01, 0x81,
  0x01, 0x26, 0xFF, 0x00, 0x46, 0xFF, 0x00, 0x09, 0x30, 0x09, 0x31, 0x09, 0x32, 0x09, 0x35, 0x75,
  0x08, 0x95, 0x04, 0x81, 0x02, 0x06, 0x00, 0xFF, 0x09, 0x20, 0x95, 0x01, 0x81, 0x02, 0xC0
};

Adafruit_USBD_HID usb_hid;

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

// リカバリ判定用
static bool was_mounted = false;

// ==========================================
// 3) メインロジック
// ==========================================

void setup() {
  watchdog_enable(2000, 1);

  // USB CDC (デバッグ用シリアル) 開始
  // これにより PC 上でシリアルモニタを開くとログが見れます
  Serial.begin(115200);

  // USB Device 設定
  TinyUSBDevice.detach(); 
  TinyUSBDevice.setID(0x0F0D, 0x00C1);
  TinyUSBDevice.setManufacturerDescriptor("HORI");
  TinyUSBDevice.setProductDescriptor("HORIPAD");

  usb_hid.setReportDescriptor(hid_report_desc, sizeof(hid_report_desc));
  usb_hid.setPollInterval(1);
  usb_hid.begin();

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

  // ------------------------------------------
  // 0. USB 接続状態監視 & リカバリ
  // ------------------------------------------
  bool is_mounted = TinyUSBDevice.mounted();
  if (was_mounted && !is_mounted) {
    // 切断された瞬間
    Serial.println("USB Unmounted (Disconnected)");
    // 安全のため入力をリセット
    gp_report.buttons = 0;
    gp_report.hat = 0x08;
    current_led_state = LED_DISCONNECT;
  } else if (!was_mounted && is_mounted) {
    // 接続された瞬間
    Serial.println("USB Mounted (Connected)");
    current_led_state = LED_IDLE;
  }
  was_mounted = is_mounted;

  // ------------------------------------------
  // 1. UART 受信処理 (デバッグ出力付き)
  // ------------------------------------------
  int available_bytes = Serial1.available();
  while (available_bytes > 0) {
    char c = (char)Serial1.read();
    available_bytes--;

    if (c == '\n' || c == '\r') {
      if (rx_index > 0) {
        rx_buffer[rx_index] = '\0';
        
        // パース処理
        parse_protocol_line(rx_buffer);
        rx_index = 0; 
        
        // コマンド受信時のみ更新 (parse_protocol_line内で成功判定しても良い)
        last_command_ms = millis();
        current_led_state = LED_ACTIVE;
      }
    } else {
      if (rx_index < RX_BUFFER_SIZE - 1) {
        rx_buffer[rx_index++] = c;
      } else {
        // オーバーフロー発生
        Serial.println("Error: RX Buffer Overflow!");
        rx_index = 0;
        
        // エラーLED点滅開始 (500ms継続)
        current_led_state = LED_ERROR;
        error_blink_start = millis();
      }
    }
  }

  // ------------------------------------------
  // 2. 安全装置 & LED制御
  // ------------------------------------------
  
  // エラー表示中はタイムアウト判定をスキップして点滅優先
  if (current_led_state == LED_ERROR) {
    if (millis() - error_blink_start > 500) {
      // エラー表示終了後は接続状態に合わせて戻る
      current_led_state = is_mounted ? LED_IDLE : LED_DISCONNECT;
    }
  } 
  else if (millis() - last_command_ms > 250) {
    // タイムアウト (通信なし)
    if (gp_report.buttons != 0 || gp_report.hat != 0x08) {
       Serial.println("Timeout: Resetting to neutral");
       gp_report.buttons = 0;
       gp_report.hat = 0x08; 
       gp_report.lx = 0x80; gp_report.ly = 0x80;
       gp_report.rx = 0x80; gp_report.ry = 0x80;
       rx_index = 0; 
    }
    
    // 接続状態に応じてLEDを戻す (アクティブ状態からの復帰)
    if (current_led_state == LED_ACTIVE) {
        current_led_state = is_mounted ? LED_IDLE : LED_DISCONNECT;
    }
    // 未接続なら赤のままにする
    else if (!is_mounted) {
        current_led_state = LED_DISCONNECT;
    }
  }

  update_led();

  // ------------------------------------------
  // 3. HID 送信
  // ------------------------------------------
  static uint32_t last_ms = 0;
  uint32_t now = millis();
  if (now - last_ms >= 8) {
    last_ms = now;
    if (is_mounted && usb_hid.ready()) {
      usb_hid.sendReport(0, &gp_report, sizeof(gp_report));
      
      // タイミング微調整 (delayMicroseconds)
      // Switch側が取りこぼす場合の調整弁
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
    case LED_DISCONNECT:
      color = neopixel.Color(50, 0, 0); // 赤
      break;
    case LED_IDLE:
      color = neopixel.Color(0, 0, 50); // 青
      break;
    case LED_ACTIVE:
      color = neopixel.Color(0, 50, 0); // 緑
      break;
    case LED_ERROR:
      // 赤点滅 (100ms周期)
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

// プロトコル解析関数
static void parse_protocol_line(char* line) {
  if (strlen(line) < 4) return;
  
  // 拡張コマンド対応: 先頭が16進数でない場合は無視する
  // Poke-Controller Modified Extensionなどが特殊コマンド(M=...)などを送ってくる場合の対策
  if (!is_hex_char(line[0])) {
    Serial.print("Ignored Unknown Cmd: ");
    Serial.println(line);
    return;
  }

  char* p = line;
  
  // デバッグ用: 受信した生データをシリアルに出力 (コメントアウトしてもOK)
  // Serial.printf("RX: %s\n", line);

  // 1: Buttons
  uint16_t raw_btns = (uint16_t)strtoul(p, &p, 16);
  while (*p == ' ') p++;
  
  // 2: Hat
  uint8_t hat = (uint8_t)strtoul(p, &p, 16);
  while (*p == ' ') p++;
  
  // 3-6: Sticks
  uint8_t lx = (uint8_t)strtoul(p, &p, 16);
  while (*p == ' ') p++;
  uint8_t ly = (uint8_t)strtoul(p, &p, 16);
  while (*p == ' ') p++;
  uint8_t rx = (uint8_t)strtoul(p, &p, 16);
  while (*p == ' ') p++;
  uint8_t ry = (uint8_t)strtoul(p, &p, 16);

  // ボタンデータを2bit右にシフトして元の位置に戻す
  gp_report.buttons = raw_btns >> 2;
  gp_report.hat = hat;

  // スティックの有効/無効フラグを判定
  bool use_right = raw_btns & 0x01;
  bool use_left  = raw_btns & 0x02;

  if (use_left) {
    gp_report.lx = lx; gp_report.ly = ly;
  } else {
    gp_report.lx = 0x80; gp_report.ly = 0x80;
  }

  if (use_right) {
    gp_report.rx = rx; gp_report.ry = ry;
  } else {
    gp_report.rx = 0x80; gp_report.ry = 0x80;
  }
}
