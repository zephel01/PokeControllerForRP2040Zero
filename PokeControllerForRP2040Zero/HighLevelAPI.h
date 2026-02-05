/**
 * HighLevelAPI.h - 高レベルAPI
 * v1.4.0: スティック操作・ボタン操作の高レベルAPI追加
 */

#ifndef HIGHLEVELAPI_H
#define HIGHLEVELAPI_H

#include <Arduino.h>

// スティック方向列挙型（左スティック）
typedef enum {
  LS_CENTER,      // 中央
  LS_UP,          // 上
  LS_UP_RIGHT,    // 右上
  LS_RIGHT,       // 右
  LS_DOWN_RIGHT,  // 右下
  LS_DOWN,        // 下
  LS_DOWN_LEFT,   // 左下
  LS_LEFT,        // 左
  LS_UP_LEFT      // 左上
} LeftStickDirection;

// スティック方向列挙型（右スティック）
typedef enum {
  RS_CENTER,      // 中央
  RS_UP,          // 上
  RS_UP_RIGHT,    // 右上
  RS_RIGHT,       // 右
  RS_DOWN_RIGHT,  // 右下
  RS_DOWN,        // 下
  RS_DOWN_LEFT,   // 左下
  RS_LEFT,        // 左
  RS_UP_LEFT      // 左上
} RightStickDirection;

// ボタンコマンド列挙型
typedef enum {
  CMD_UP,        // Lスティック上
  CMD_DOWN,      // Lスティック下
  CMD_LEFT,      // Lスティック左
  CMD_RIGHT,     // Lスティック右
  CMD_RS_UP,     // Rスティック上
  CMD_RS_DOWN,   // Rスティック下
  CMD_RS_LEFT,   // Rスティック左
  CMD_RS_RIGHT,  // Rスティック右
  CMD_A,         // Aボタン
  CMD_B,         // Bボタン
  CMD_X,         // Xボタン
  CMD_Y,         // Yボタン
  CMD_L,         // Lボタン
  CMD_R,         // Rボタン
  CMD_ZL,        // ZLボタン
  CMD_ZR,        // ZRボタン
  CMD_PLUS,      // +ボタン
  CMD_MINUS,     // -ボタン
  CMD_HOME,      // HOMEボタン
  CMD_CAPTURE,   // キャプチャボタン
} ButtonCommand;

// 外部関数宣言
void pushButton(ButtonCommand cmd, int delay_after_pushing_msec, int loop_num);
void pushButton2(ButtonCommand cmd, int pushing_time_msec, int delay_after_pushing_msec, int loop_num);
void pushHatButton(uint8_t hat_value, int delay_after_pushing_msec, int loop_num);
void pushHatButtonContinuous(uint8_t hat_value, int pushing_time_msec);
void tiltJoystick(int lx_per, int ly_per, int rx_per, int ry_per, 
                  int tilt_time_msec, int delay_after_tilt_msec);
void useLeftStick(LeftStickDirection dir, int tilt_time_msec, int delay_after_tilt_msec);
void useRightStick(RightStickDirection dir, int tilt_time_msec, int delay_after_tilt_msec);
void tiltLeftStick(int direction_deg, double power, int holdtime, int delaytime);

#endif // HIGHLEVELAPI_H
