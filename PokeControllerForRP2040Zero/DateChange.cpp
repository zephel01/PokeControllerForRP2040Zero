/**
 * DateChange.cpp - 日付・年変更実装
 */

#include "DateChange.h"
#include "Common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// タイミング定数（ms）
#define MENU_DELAY    200
#define CONFIRM_DELAY 500
#define ANIM_DELAY    1000

// Date Y/M/D コマンド解析
// 例: "Date 1/0/1" (1年0ヶ月1日進める)
// 例: "Date -1/0/0" (1年戻す)
bool parse_date_command(const char* cmd, DateChangeCommand* out_cmd) {
  if (strncmp(cmd, "Date ", 5) != 0) return false;
  
  char year_str[8], month_str[8], day_str[8];
  int parsed = sscanf(cmd + 5, "%7[^/]/%7[^/]/%7s", year_str, month_str, day_str);
  
  if (parsed != 3) return false;
  
  out_cmd->year_delta = atoi(year_str);
  out_cmd->month_delta = atoi(month_str);
  out_cmd->day_delta = atoi(day_str);
  out_cmd->repeat_count = abs(out_cmd->year_delta) + abs(out_cmd->month_delta) + abs(out_cmd->day_delta);
  
  Serial.printf("Date Command: Y=%d, M=%d, D=%d\n", 
                out_cmd->year_delta, out_cmd->month_delta, out_cmd->day_delta);
  return true;
}

// Year N コマンド解析
// 例: "Year 5" (5年進める)
// 例: "Year -3" (3年戻す)
bool parse_year_command(const char* cmd, int* out_years) {
  if (strncmp(cmd, "Year ", 5) != 0) return false;
  
  *out_years = atoi(cmd + 5);
  
  if (*out_years == 0) return false;
  
  Serial.printf("Year Command: %d years\n", *out_years);
  return true;
}

// 日付変更実行（Switch設定画面操作）
void execute_date_change(const DateChangeCommand* cmd) {
  // HOME → 設定 → 日付設定 → 日付変更
  uint32_t delay_time = MENU_DELAY;
  
  // HOMEボタン押下
  press_button(BUTTON_HOME, delay_time);
  delay(ANIM_DELAY);
  
  // 右へ（設定メニューへ）
  press_hat(HAT_RIGHT, delay_time);
  delay(delay_time);
  
  // 下へ（日付・時刻設定へ）
  press_hat(HAT_DOWN, delay_time);
  delay(delay_time);
  
  // Aで選択
  press_button(BUTTON_A, delay_time);
  delay(ANIM_DELAY);
  
  // 下へ（日付変更へ）
  press_hat(HAT_DOWN, delay_time);
  delay(delay_time);
  
  // Aで日付変更画面へ
  press_button(BUTTON_A, delay_time);
  delay(ANIM_DELAY);
  
  // 年の変更
  int year_steps = abs(cmd->year_delta);
  uint8_t year_dir = (cmd->year_delta > 0) ? HAT_RIGHT : HAT_LEFT;
  for (int i = 0; i < year_steps; i++) {
    press_hat(year_dir, delay_time);
    delay(delay_time / 2);
  }
  
  // 月の変更
  int month_steps = abs(cmd->month_delta);
  uint8_t month_dir = (cmd->month_delta > 0) ? HAT_RIGHT : HAT_LEFT;
  for (int i = 0; i < month_steps; i++) {
    press_hat(month_dir, delay_time);
    delay(delay_time / 2);
  }
  
  // 日の変更
  int day_steps = abs(cmd->day_delta);
  uint8_t day_dir = (cmd->day_delta > 0) ? HAT_RIGHT : HAT_LEFT;
  for (int i = 0; i < day_steps; i++) {
    press_hat(day_dir, delay_time);
    delay(delay_time / 2);
  }
  
  // Aで確定
  press_button(BUTTON_A, delay_time);
  delay(ANIM_DELAY);
  
  // HOMEに戻る
  press_button(BUTTON_HOME, delay_time);
}

// 年変更実装（最大60年）
#define MAX_YEAR_CHANGE 60

void execute_year_change(int years) {
  if (abs(years) > MAX_YEAR_CHANGE) {
    Serial.printf("Year change limited to %d years\n", MAX_YEAR_CHANGE);
    years = (years > 0) ? MAX_YEAR_CHANGE : -MAX_YEAR_CHANGE;
  }
  
  DateChangeCommand cmd;
  cmd.year_delta = years;
  cmd.month_delta = 0;
  cmd.day_delta = 0;
  cmd.repeat_count = abs(years);
  
  execute_date_change(&cmd);
}
