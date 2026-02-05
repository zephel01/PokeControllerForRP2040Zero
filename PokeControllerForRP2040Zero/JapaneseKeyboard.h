/**
 * JapaneseKeyboard.h - 日本語キーボードレイアウト
 * v1.4.0: 日本語キーボード（JIS）対応追加
 */

#ifndef JAPANESEKEYBOARD_H
#define JAPANESEKEYBOARD_H

#include <Arduino.h>

// 日本語キー定義（HID Usage ID）
#define KEY_JP_YEN           0x89  // ￥キー
#define KEY_JP_HENKAN        0x8A  // 変換
#define KEY_JP_MUHENKAN      0x8B  // 無変換
#define KEY_JP_COLON         0x8E  // ：（コロン）
#define KEY_JP_AT            0x8F  // ＠
#define KEY_JP_CARET         0x90  // ＾
#define KEY_JP_BACKSLASH     0x91  // ＼

// 修飾キー定義（左右区別）
#define MOD_LEFT_CTRL       0x01
#define MOD_LEFT_SHIFT      0x02
#define MOD_LEFT_ALT        0x04
#define MOD_LEFT_GUI        0x08
#define MOD_RIGHT_CTRL      0x10
#define MOD_RIGHT_SHIFT     0x20
#define MOD_RIGHT_ALT       0x40
#define MOD_RIGHT_GUI       0x80

// 日本語キーボードのASCII→HIDマップ（簡易版）
extern const uint8_t jp_ascii_to_hid[128];
extern const uint8_t jp_ascii_shift_to_hid[128];

// 外部関数宣言
uint8_t jp_ascii_to_hid_key(char c, bool* need_shift);
void type_jp_string(const char* str);
void press_jp_key(uint8_t keycode, uint8_t modifiers);
void release_all_jp_keys(void);

#endif // JAPANESEKEYBOARD_H
