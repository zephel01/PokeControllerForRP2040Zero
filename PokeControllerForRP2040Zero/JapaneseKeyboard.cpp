/**
 * JapaneseKeyboard.cpp - 日本語キーボード実装
 */

#include "JapaneseKeyboard.h"
#include "Common.h"

// 日本語キーボードASCII→HIDマップ（シフトなし）
const uint8_t jp_ascii_to_hid[128] = {
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  // 0x00-0x0F
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  // 0x10-0x1F
  0x2C, 0x1E, 0x34, 0x20, 0x21, 0x22, 0x24, 0x25, 0x26, 0x27, 0x33, 0x38, 0x89, 0x2A, 0x1D, 0x35, // 0x20-0x2F (スペース-スラッシュ, ￥)
  0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x8E, 0x2D, // 0x30-0x3F (0-9, コロン, セミコロン)
  0x2E, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2D, // 0x40-0x4F (@-O)
  0x2E, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, // 0x50-0x5F (P-`)
   0,    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0,    // 0x60-0x6F (a-o)
  0,    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0     // 0x70-0x7F (p-DEL)
};

// 日本語キーボードASCII→HIDマップ（シフトあり）
const uint8_t jp_ascii_shift_to_hid[128] = {
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  // 0x00-0x0F
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  // 0x10-0x1F
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  // 0x20-0x2F
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  // 0x30-0x3F
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  // 0x40-0x4F
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  // 0x50-0x5F
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  // 0x60-0x6F
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0   // 0x70-0x7F
};

// ASCII文字をJISレイアウトのHIDキーコードに変換
uint8_t jp_ascii_to_hid_key(char c, bool* need_shift) {
  *need_shift = false;
  
  if (c >= 32 && c < 127) {
    // JISレイアウトでシフトが必要な文字
    switch (c) {
      case '!': case '"': case '#': case '$': case '%':
      case '&': case '\'': case '(': case ')': case '*':
      case '+': case ':': case '<': case '>': case '?':
      case '@': case '^': case '_':
        *need_shift = true;
        return jp_ascii_shift_to_hid[(uint8_t)c];
      default:
        return jp_ascii_to_hid[(uint8_t)c];
    }
  }
  return 0;
}

// 日本語文字列入力
void type_jp_string(const char* str) {
  for (int i = 0; str[i] != '\0'; i++) {
    bool need_shift = false;
    uint8_t keycode = jp_ascii_to_hid_key(str[i], &need_shift);
    
    if (keycode != 0) {
      uint8_t keys[6] = {keycode, 0, 0, 0, 0, 0};
      uint8_t modifier = need_shift ? MOD_LEFT_SHIFT : 0;
      
      usb_keyboard.keyboardReport(0, modifier, keys);
      delay(20);
      usb_keyboard.keyboardRelease(0);
      delay(20);
    }
  }
}

// 日本語キー押下（修飾キー対応）
void press_jp_key(uint8_t keycode, uint8_t modifiers) {
  uint8_t keys[6] = {keycode, 0, 0, 0, 0, 0};
  usb_keyboard.keyboardReport(0, modifiers, keys);
}

// 全キー解放
void release_all_jp_keys(void) {
  usb_keyboard.keyboardRelease(0);
}
