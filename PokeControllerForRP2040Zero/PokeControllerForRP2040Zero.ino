#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <USB.h>

/**
 * RP2040-Zero (Waveshare) 用 Switch コントローラー雛形
 * 
 * 基盤: Waveshare RP2040 Zero
 * USB Stack: Adafruit TinyUSB
 * UART: GP0 (TX), GP1 (RX) - 115200bps (Poke-Controller Modified 標準)
 * 
 * このスケッチは、Poke-Controller Modified のシリアル通信プロトコルを解釈し、
 * Switch の有線コントローラー (HORIPAD) として動作する骨格を提供します。
 */

// ==========================================
// 1) UART 設定 (RP2040-Zero: GP0/GP1)
// ==========================================
static constexpr int UART_TX_PIN = 0;  // GP0
static constexpr int UART_RX_PIN = 1;  // GP1
static constexpr uint32_t UART_BAUD = 115200; // Poke-Controller Modified のデフォルトに合わせる

// ==========================================
// 2) Switch コントローラ用 HID レポート設定
// ==========================================

// Switch が期待する HID Report Descriptor (HORI Pokken/Gamepad 互換)
uint8_t const hid_report_desc[] = {
  0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
  0x09, 0x05,        // Usage (Game Pad)
  0xA1, 0x01,        // Collection (Application)
  0x15, 0x00,        //   Logical Minimum (0)
  0x25, 0x01,        //   Logical Maximum (1)
  0x35, 0x00,        //   Physical Minimum (0)
  0x45, 0x01,        //   Physical Maximum (1)
  0x75, 0x01,        //   Report Size (1)
  0x95, 0x10,        //   Report Count (16)
  0x05, 0x09,        //   Usage Page (Button)
  0x19, 0x01,        //   Usage Minimum (0x01)
  0x29, 0x10,        //   Usage Maximum (0x10)
  0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
  0x25, 0x07,        //   Logical Maximum (7)
  0x46, 0x3B, 0x01,  //   Physical Maximum (315)
  0x75, 0x04,        //   Report Size (4)
  0x95, 0x01,        //   Report Count (1)
  0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
  0x09, 0x39,        //   Usage (Hat switch)
  0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
  0x65, 0x00,        //   Unit (None)
  0x95, 0x01,        //   Report Count (1)
  0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0x26, 0xFF, 0x00,  //   Logical Maximum (255)
  0x46, 0xFF, 0x00,  //   Physical Maximum (255)
  0x09, 0x30,        //   Usage (X)
  0x09, 0x31,        //   Usage (Y)
  0x09, 0x32,        //   Usage (Z)
  0x09, 0x35,        //   Usage (Rz)
  0x75, 0x08,        //   Report Size (8)
  0x95, 0x04,        //   Report Count (4)
  0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
  0x09, 0x20,        //   Usage (0x20)
  0x95, 0x01,        //   Report Count (1)
  0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0xC0               // End Collection
};

Adafruit_USBD_HID usb_hid;

// Switch用レポート構造体
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

// ==========================================
// 3) 入力プロトコル解釈 (Poke-Controller Modified 用)
// ==========================================

static char serial_buf[64];
static int  serial_idx = 0;

// 16進数文字を数値に変換
static uint32_t hex2int(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0;
}

// 文字列から16進数数値を読み取る
static uint32_t parse_hex(const char* p, int len) {
  uint32_t val = 0;
  for (int i = 0; i < len; i++) {
    val = (val << 4) | hex2int(p[i]);
  }
  return val;
}

/**
 * Poke-Controller Modified プロトコル解釈
 * フォーマット例: "0004 08 80 80 80 80\n" (Btns, Hat, LX, LY, RX, RY の16進スペー隔て)
 * ※ btns の下位2bitは Stick使用フラグ (bit0:Right, bit1:Left) の場合が多い
 */
static void parse_protocol_line(char* line) {
  // 簡易的なスペース区切りパース
  char* p = line;
  
  // 1: Buttons (4 hex chars)
  uint16_t raw_btns = (uint16_t)strtoul(p, &p, 16);
  while (*p == ' ') p++;
  
  // 2: Hat (2 hex chars)
  uint8_t hat = (uint8_t)strtoul(p, &p, 16);
  while (*p == ' ') p++;
  
  // 3: LX (2 hex chars)
  uint8_t lx = (uint8_t)strtoul(p, &p, 16);
  while (*p == ' ') p++;
  
  // 4: LY (2 hex chars)
  uint8_t ly = (uint8_t)strtoul(p, &p, 16);
  while (*p == ' ') p++;
  
  // 5: RX (2 hex chars)
  uint8_t rx = (uint8_t)strtoul(p, &p, 16);
  while (*p == ' ') p++;
  
  // 6: RY (2 hex chars)
  uint8_t ry = (uint8_t)strtoul(p, &p, 16);

  // フラグ処理 (PokeControllerForPico の実装を参考)
  // btns >>= 2 が実際のボタン、下位2bitがスティック有効フラグの場合
  gp_report.buttons = raw_btns >> 2;
  gp_report.hat = hat;
  
  bool use_right = raw_btns & 0x01;
  bool use_left  = raw_btns & 0x02;

  if (use_left) {
    gp_report.lx = lx;
    gp_report.ly = ly;
  } else {
    gp_report.lx = 0x80;
    gp_report.ly = 0x80;
  }

  if (use_right && use_left) {
    gp_report.rx = rx;
    gp_report.ry = ry;
  } else if (use_right) {
    // Leftの値が送られてきている場合を考慮
    gp_report.rx = lx;
    gp_report.ry = ly;
  } else {
    gp_report.rx = 0x80;
    gp_report.ry = 0x80;
  }
}

// ==========================================
// 4) メインロジック
// ==========================================

void setup() {
  // 1. USB VID/PID 設定 (HORIPAD 互換)
  USB.disconnect();
  USB.setVID(0x0F0D);
  USB.setPID(0x00C1); // HORIPAD for Nintendo Switch
  USB.setManufacturerString("HORI");
  USB.setProductString("HORIPAD");
  USB.connect();
  delay(500);

  // 2. UART 初期化
  Serial1.setTX(UART_TX_PIN);
  Serial1.setRX(UART_RX_PIN);
  Serial1.begin(UART_BAUD);

  // 3. HID 初期化
  usb_hid.setReportDescriptor(hid_report_desc, sizeof(hid_report_desc));
  usb_hid.setPollInterval(1);
  usb_hid.begin();

  // LED 設定 (動作確認用)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  // --- UART 受信 ---
  while (Serial1.available() > 0) {
    char c = (char)Serial1.read();
    if (c == '\n' || c == '\r') {
      if (serial_idx > 0) {
        serial_buf[serial_idx] = '\0';
        parse_protocol_line(serial_buf);
        serial_idx = 0;
      }
    } else if (serial_idx < (int)sizeof(serial_buf) - 1) {
      serial_buf[serial_idx++] = c;
    }
  }

  // --- HID 送信 (約2ms周期) ---
  static uint32_t last_ms = 0;
  uint32_t now = millis();
  if (now - last_ms >= 2) {
    last_ms = now;
    if (usb_hid.ready()) {
      usb_hid.sendReport(0, &gp_report, sizeof(gp_report));
    }
  }
  
  // 接続中なら LED を点滅させるなどの処理
  digitalWrite(LED_BUILTIN, usb_hid.ready() ? HIGH : LOW);
}
