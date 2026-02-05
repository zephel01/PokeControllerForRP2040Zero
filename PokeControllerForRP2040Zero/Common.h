/**
 * Common.h - 共通定義・ヘルパー関数
 * v1.4.0: 重複コードの統合
 */

#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

// ==========================================
// Gamepadレポート定義
// ==========================================
typedef struct TU_ATTR_PACKED {
  uint16_t buttons;
  uint8_t  hat;
  uint8_t  lx;
  uint8_t  ly;
  uint8_t  rx;
  uint8_t  ry;
  uint8_t  vendor;
} switch_report_t;

// ==========================================
// 外部変数宣言（メインファイルで定義）
// ==========================================
extern Adafruit_USBD_HID usb_gamepad;
extern Adafruit_USBD_HID usb_keyboard;
extern switch_report_t gp_report;

// ==========================================
// ボタン定義（ビットマップ）
// ==========================================
#define BUTTON_Y       0x0001
#define BUTTON_B       0x0002
#define BUTTON_A       0x0004
#define BUTTON_X       0x0008
#define BUTTON_L       0x0010
#define BUTTON_R       0x0020
#define BUTTON_ZL      0x0040
#define BUTTON_ZR      0x0080
#define BUTTON_MINUS   0x0100
#define BUTTON_PLUS    0x0200
#define BUTTON_LCLICK  0x0400
#define BUTTON_RCLICK  0x0800
#define BUTTON_HOME    0x1000
#define BUTTON_CAPTURE 0x2000

// ==========================================
// HATスイッチ定義
// ==========================================
#define HAT_UP      0x00
#define HAT_UP_RIGHT 0x01
#define HAT_RIGHT   0x02
#define HAT_DOWN_RIGHT 0x03
#define HAT_DOWN    0x04
#define HAT_DOWN_LEFT 0x05
#define HAT_LEFT    0x06
#define HAT_UP_LEFT 0x07
#define HAT_CENTER  0x08

// ==========================================
// スティック定数
// ==========================================
#define STICK_MIN     0
#define STICK_CENTER  128
#define STICK_MAX     255

// ==========================================
// 共通ヘルパー関数
// ==========================================

/**
 * ボタンを押して離す
 * @param btn ボタンのビットマップ（BUTTON_Aなど）
 * @param duration 押下時間（ミリ秒）
 */
static inline void press_button(uint16_t btn, uint16_t duration) {
  gp_report.buttons |= btn;
  if (usb_gamepad.ready()) {
    usb_gamepad.sendReport(0, &gp_report, sizeof(gp_report));
  }
  delay(duration);
  gp_report.buttons &= ~btn;
}

/**
 * HATスイッチを操作
 * @param hat_val HAT値（HAT_UPなど）
 * @param duration 操作時間（ミリ秒）
 */
static inline void press_hat(uint8_t hat_val, uint16_t duration) {
  gp_report.hat = hat_val;
  if (usb_gamepad.ready()) {
    usb_gamepad.sendReport(0, &gp_report, sizeof(gp_report));
  }
  delay(duration);
  gp_report.hat = HAT_CENTER; // 中央
}

/**
 * Gamepadレポートを送信
 */
static inline void send_report(void) {
  if (usb_gamepad.ready()) {
    usb_gamepad.sendReport(0, &gp_report, sizeof(gp_report));
  }
}

/**
 * パーセント値をスティック値に変換
 * @param percent -100 ~ 100
 * @return 0 ~ 255（128が中央）
 */
static inline uint8_t percent_to_value(int percent) {
  int value = STICK_CENTER + (percent * 127 / 100);
  if (value < STICK_MIN) value = STICK_MIN;
  if (value > STICK_MAX) value = STICK_MAX;
  return (uint8_t)value;
}

#endif // COMMON_H
