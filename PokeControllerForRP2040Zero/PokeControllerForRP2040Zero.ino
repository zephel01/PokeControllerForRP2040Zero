#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

/**
 * RP2040-Zero (Waveshare) 用 Switch コントローラー雛形
 */

// ==========================================
// 1) UART 設定 (RP2040-Zero: GP0/GP1)
// ==========================================
static constexpr int UART_TX_PIN = 0;  // GP0
static constexpr int UART_RX_PIN = 1;  // GP1
static constexpr uint32_t UART_BAUD = 115200; 

// ==========================================
// 2) Switch コントローラ用 HID レポート設定
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

// ==========================================
// 3) UART バッファ設定
// ==========================================
static char serial_buf[64];
static int  serial_idx = 0;
static uint32_t last_command_ms = 0; // 最終コマンド受信時刻

// ==========================================
// 4) メインロジック
// ==========================================

void setup() {
  // USB 初期化と設定の反映
  TinyUSBDevice.detach(); // 一旦切断して再認識を促す
  TinyUSBDevice.setID(0x0F0D, 0x00C1);
  TinyUSBDevice.setManufacturerDescriptor("HORI");
  TinyUSBDevice.setProductDescriptor("HORIPAD");

  usb_hid.setReportDescriptor(hid_report_desc, sizeof(hid_report_desc));
  usb_hid.setPollInterval(1);
  usb_hid.begin();

  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin();
  }

  // 設定反映のための待機と再接続
  delay(1000);
  TinyUSBDevice.attach();

  // UART 初期化
  Serial1.setTX(UART_TX_PIN);
  Serial1.setRX(UART_RX_PIN);
  Serial1.begin(UART_BAUD);
}

void loop() {
  // UART 受信処理
  while (Serial1.available() > 0) {
    char c = (char)Serial1.read();
    if (c == '\n' || c == '\r') {
      if (serial_idx > 0) {
        serial_buf[serial_idx] = '\0';
        parse_protocol_line(serial_buf);
        serial_idx = 0;
        last_command_ms = millis(); // コマンド受信時刻を更新
      }
    } else if (serial_idx < (int)sizeof(serial_buf) - 1) {
      serial_buf[serial_idx++] = c;
    }
  }

  // 安全装置: 一定時間コマンドが来なければニュートラルに戻す (250ms)
  if (millis() - last_command_ms > 250) {
    if (gp_report.buttons != 0 || gp_report.hat != 0x08) {
       gp_report.buttons = 0;
       gp_report.hat = 0x08; // 0x08 = Center
       gp_report.lx = 0x80;  // 0x80 = Center
       gp_report.ly = 0x80;
       gp_report.rx = 0x80;
       gp_report.ry = 0x80;
       
       // 通信途絶とみなしてバッファもクリア（ゴミデータ対策）
       serial_idx = 0; 
    }
  }

  // HID 送信処理 (8ms周期)
  static uint32_t last_ms = 0;
  uint32_t now = millis();
  if (now - last_ms >= 8) {
    last_ms = now;
    if (TinyUSBDevice.mounted() && usb_hid.ready()) {
      usb_hid.sendReport(0, &gp_report, sizeof(gp_report));
    }
  }
}

// プロトコル解析関数
static void parse_protocol_line(char* line) {
  // 簡易チェック: 文字列が短すぎる場合は無視
  // "0000 08..." 最低でもこれくらいの長さはあるはず
  if (strlen(line) < 4) return;

  char* p = line;
  
  // 1: Buttons (4 hex chars)
  // 下位2bit: 0=RightStick, 1=LeftStick の有効フラグ
  // 上位14bit: ボタンデータ
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

  // ボタンデータを2bit右にシフトして元の位置に戻す
  gp_report.buttons = raw_btns >> 2;
  gp_report.hat = hat;

  // スティックの有効/無効フラグを判定
  bool use_right = raw_btns & 0x01;
  bool use_left  = raw_btns & 0x02;

  // 左スティック処理
  if (use_left) {
    gp_report.lx = lx;
    gp_report.ly = ly;
  } else {
    gp_report.lx = 0x80; // Center
    gp_report.ly = 0x80; // Center
  }

  // 右スティック処理
  if (use_right) {
    gp_report.rx = rx;
    gp_report.ry = ry;
  } else {
    gp_report.rx = 0x80; // Center
    gp_report.ry = 0x80; // Center
  }
}
