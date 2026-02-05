/**
 * DateChange.h - 日付・年変更機能
 * v1.4.0: 日付・年変更コマンド追加
 */

#ifndef DATECHANGE_H
#define DATECHANGE_H

#include <Arduino.h>

// 日付変更コマンド構造体
typedef struct {
  int year_delta;    // 年の変化量（正: 増加, 負: 減少）
  int month_delta;   // 月の変化量
  int day_delta;     // 日の変化量
  int repeat_count;   // 繰り返し回数
} DateChangeCommand;

// 外部関数宣言
bool parse_date_command(const char* cmd, DateChangeCommand* out_cmd);
bool parse_year_command(const char* cmd, int* out_years);
void execute_date_change(const DateChangeCommand* cmd);
void execute_year_change(int years);

#endif // DATECHANGE_H
