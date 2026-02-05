/**
 * HighLevelAPI.cpp - 高レベルAPI実装
 */

#include "HighLevelAPI.h"
#include "Common.h"
#include <math.h>

// ボタンコマンドをビットマップに変換
static uint16_t cmd_to_bitmap(ButtonCommand cmd) {
  switch (cmd) {
    case CMD_A:       return BUTTON_A;
    case CMD_B:       return BUTTON_B;
    case CMD_X:       return BUTTON_X;
    case CMD_Y:       return BUTTON_Y;
    case CMD_L:       return BUTTON_L;
    case CMD_R:       return BUTTON_R;
    case CMD_ZL:      return BUTTON_ZL;
    case CMD_ZR:      return BUTTON_ZR;
    case CMD_PLUS:    return BUTTON_PLUS;
    case CMD_MINUS:   return BUTTON_MINUS;
    case CMD_HOME:    return BUTTON_HOME;
    case CMD_CAPTURE: return BUTTON_CAPTURE;
    default:          return 0;
  }
}

// ==================== 高レベルAPI実装 ====================

// ボタンを押して離す（デフォルト押下時間）
void pushButton(ButtonCommand cmd, int delay_after_pushing_msec, int loop_num) {
  for (int i = 0; i < loop_num; i++) {
    if (cmd >= CMD_UP && cmd <= CMD_RS_RIGHT) {
      // スティックコマンド
      switch (cmd) {
        case CMD_UP:    useLeftStick(LS_UP, 50, delay_after_pushing_msec); break;
        case CMD_DOWN:  useLeftStick(LS_DOWN, 50, delay_after_pushing_msec); break;
        case CMD_LEFT:  useLeftStick(LS_LEFT, 50, delay_after_pushing_msec); break;
        case CMD_RIGHT: useLeftStick(LS_RIGHT, 50, delay_after_pushing_msec); break;
        case CMD_RS_UP:    useRightStick(RS_UP, 50, delay_after_pushing_msec); break;
        case CMD_RS_DOWN:  useRightStick(RS_DOWN, 50, delay_after_pushing_msec); break;
        case CMD_RS_LEFT:  useRightStick(RS_LEFT, 50, delay_after_pushing_msec); break;
        case CMD_RS_RIGHT: useRightStick(RS_RIGHT, 50, delay_after_pushing_msec); break;
      }
    } else {
      // ボタンコマンド
      uint16_t btn = cmd_to_bitmap(cmd);
      gp_report.buttons |= btn;
      send_report();
      delay(50); // デフォルト押下時間
      gp_report.buttons &= ~btn;
      send_report();
      delay(delay_after_pushing_msec);
    }
  }
}

// ボタン押下時間指定版
void pushButton2(ButtonCommand cmd, int pushing_time_msec, int delay_after_pushing_msec, int loop_num) {
  for (int i = 0; i < loop_num; i++) {
    if (cmd < CMD_UP || cmd > CMD_RS_RIGHT) {
      uint16_t btn = cmd_to_bitmap(cmd);
      gp_report.buttons |= btn;
      send_report();
      delay(pushing_time_msec);
      gp_report.buttons &= ~btn;
      send_report();
      delay(delay_after_pushing_msec);
    }
  }
}

// HATボタンを押す
void pushHatButton(uint8_t hat_value, int delay_after_pushing_msec, int loop_num) {
  for (int i = 0; i < loop_num; i++) {
    gp_report.hat = hat_value;
    send_report();
    delay(50);
    gp_report.hat = 0x08;
    send_report();
    delay(delay_after_pushing_msec);
  }
}

// HATボタンを押し続ける
void pushHatButtonContinuous(uint8_t hat_value, int pushing_time_msec) {
  gp_report.hat = hat_value;
  send_report();
  delay(pushing_time_msec);
  gp_report.hat = 0x08;
  send_report();
}

// スティック傾き（パーセント指定）
void tiltJoystick(int lx_per, int ly_per, int rx_per, int ry_per,
                  int tilt_time_msec, int delay_after_tilt_msec) {
  gp_report.lx = percent_to_value(lx_per);
  gp_report.ly = percent_to_value(ly_per);
  gp_report.rx = percent_to_value(rx_per);
  gp_report.ry = percent_to_value(ry_per);
  
  send_report();
  delay(tilt_time_msec);
  
  // 中央に戻す
  gp_report.lx = STICK_CENTER;
  gp_report.ly = STICK_CENTER;
  gp_report.rx = STICK_CENTER;
  gp_report.ry = STICK_CENTER;
  
  send_report();
  delay(delay_after_tilt_msec);
}

// 左スティック傾け（8方向）
void useLeftStick(LeftStickDirection dir, int tilt_time_msec, int delay_after_tilt_msec) {
  int lx = STICK_CENTER, ly = STICK_CENTER;
  
  switch (dir) {
    case LS_UP:         lx = STICK_CENTER; ly = STICK_MIN; break;
    case LS_UP_RIGHT:   lx = STICK_MAX; ly = STICK_MIN; break;
    case LS_RIGHT:      lx = STICK_MAX; ly = STICK_CENTER; break;
    case LS_DOWN_RIGHT: lx = STICK_MAX; ly = STICK_MAX; break;
    case LS_DOWN:       lx = STICK_CENTER; ly = STICK_MAX; break;
    case LS_DOWN_LEFT:  lx = STICK_MIN; ly = STICK_MAX; break;
    case LS_LEFT:       lx = STICK_MIN; ly = STICK_CENTER; break;
    case LS_UP_LEFT:    lx = STICK_MIN; ly = STICK_MIN; break;
    case LS_CENTER:     // 中央
    default:            lx = STICK_CENTER; ly = STICK_CENTER; break;
  }
  
  gp_report.lx = (uint8_t)lx;
  gp_report.ly = (uint8_t)ly;
  send_report();
  delay(tilt_time_msec);
  
  gp_report.lx = STICK_CENTER;
  gp_report.ly = STICK_CENTER;
  send_report();
  delay(delay_after_tilt_msec);
}

// 右スティック傾け（8方向）
void useRightStick(RightStickDirection dir, int tilt_time_msec, int delay_after_tilt_msec) {
  int rx = STICK_CENTER, ry = STICK_CENTER;
  
  switch (dir) {
    case RS_UP:         rx = STICK_CENTER; ry = STICK_MIN; break;
    case RS_UP_RIGHT:   rx = STICK_MAX; ry = STICK_MIN; break;
    case RS_RIGHT:      rx = STICK_MAX; ry = STICK_CENTER; break;
    case RS_DOWN_RIGHT: rx = STICK_MAX; ry = STICK_MAX; break;
    case RS_DOWN:       rx = STICK_CENTER; ry = STICK_MAX; break;
    case RS_DOWN_LEFT:  rx = STICK_MIN; ry = STICK_MAX; break;
    case RS_LEFT:       rx = STICK_MIN; ry = STICK_CENTER; break;
    case RS_UP_LEFT:    rx = STICK_MIN; ry = STICK_MIN; break;
    case RS_CENTER:     // 中央
    default:            rx = STICK_CENTER; ry = STICK_CENTER; break;
  }
  
  gp_report.rx = (uint8_t)rx;
  gp_report.ry = (uint8_t)ry;
  send_report();
  delay(tilt_time_msec);
  
  gp_report.rx = STICK_CENTER;
  gp_report.ry = STICK_CENTER;
  send_report();
  delay(delay_after_tilt_msec);
}

// 左スティックを角度とパワーで傾ける
void tiltLeftStick(int direction_deg, double power, int holdtime, int delaytime) {
  // direction_deg: 0=右, 90=上, 180=左, -90=下
  // power: 0.0 ~ 1.0 (1.0で最大傾き)
  
  if (power > 1.0) power = 1.0;
  if (power < 0.0) power = 0.0;
  
  double rad = direction_deg * 3.14159265 / 180.0;
  
  int lx = STICK_CENTER + (int)(sin(rad) * power * 127.0);
  int ly = STICK_CENTER - (int)(cos(rad) * power * 127.0); // Y軸は逆
  
  if (lx < STICK_MIN) lx = STICK_MIN;
  if (lx > STICK_MAX) lx = STICK_MAX;
  if (ly < STICK_MIN) ly = STICK_MIN;
  if (ly > STICK_MAX) ly = STICK_MAX;
  
  gp_report.lx = (uint8_t)lx;
  gp_report.ly = (uint8_t)ly;
  send_report();
  delay(holdtime);
  
  gp_report.lx = STICK_CENTER;
  gp_report.ly = STICK_CENTER;
  send_report();
  delay(delaytime);
}
