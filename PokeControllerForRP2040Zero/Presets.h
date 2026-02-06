/**
 * Presets.h - プリセットコマンド定義（Pico互換方式）
 * v1.4.0: プリセットコマンド機能追加
 * v1.5.0: Pico互換コマンドシステム実装
 */

#ifndef PRESETS_H
#define PRESETS_H

#include <Arduino.h>
#include "Common.h"

// 日付変更用インデックス定義
#define INDEX_ARRAY_YEAR  (30)
#define INDEX_ARRAY_MONTH (32)
#define INDEX_ARRAY_DAY   (34)

// ==========================================
// ループステート列挙型
// ==========================================
typedef enum {
  STATE0,  // シリアル通信受信→受信成功
  STATE1,  // シリアル通信成功時
  STATE2,  // 判定
} LOOPSTATE;

// ==========================================
// コマンド定義（BUTTON_DEFINE）
// ==========================================
typedef enum {
  COMMAND_NONE = 0,

  // 左スティック
  COMMAND_UP,
  COMMAND_DOWN,
  COMMAND_LEFT,
  COMMAND_RIGHT,
  COMMAND_UPLEFT,
  COMMAND_UPRIGHT,
  COMMAND_DOWNLEFT,
  COMMAND_DOWNRIGHT,

  // ボタン
  COMMAND_X,
  COMMAND_Y,
  COMMAND_A,
  COMMAND_B,
  COMMAND_L,
  COMMAND_R,
  COMMAND_ZL,
  COMMAND_ZR,
  COMMAND_TRIGGERS,
  COMMAND_PLUS,
  COMMAND_MINUS,
  COMMAND_HOME,
  COMMAND_CAPTURE,
  COMMAND_NOP,

  // 右スティック
  COMMAND_RS_UP,
  COMMAND_RS_DOWN,
  COMMAND_RS_LEFT,
  COMMAND_RS_RIGHT,
  COMMAND_RS_UPLEFT,
  COMMAND_RS_UPRIGHT,
  COMMAND_RS_DOWNLEFT,
  COMMAND_RS_DOWNRIGHT,

  // HAT
  COMMAND_HAT_TOP,
  COMMAND_HAT_TOP_RIGHT,
  COMMAND_HAT_RIGHT,
  COMMAND_HAT_BOTTOM_RIGHT,
  COMMAND_HAT_BOTTOM,
  COMMAND_HAT_BOTTOM_LEFT,
  COMMAND_HAT_LEFT,
  COMMAND_HAT_TOP_LEFT
} BUTTON_DEFINE;

// ==========================================
// プロセス状態列挙型
// ==========================================
typedef enum {
  PRESET_NONE = 0,
  PC_CALL,              // 単発コマンド
  PC_CALL_STRING,       // キーボード文字列入力
  PC_CALL_KEYBOARD,     // キーボード特殊キー
  PC_CALL_KEYBOARD_PRESS,
  PC_CALL_KEYBOARD_RELEASE,
  MASH_A,               // Aボタン連打
  AAABB,                // AAABBパターン
  AUTO_LEAGUE,          // 自動リーグ戦闘
  INF_WATT,             // 無限ワット収集
  PICKUPBERRY,          // ベリー収集
  CHANGETHEDATE,        // 日付変更
  CHANGETHEYEAR,        // 年変更
} ProcessState;

// ==========================================
// コマンド配列宣言
// ==========================================
extern const SetCommand mash_a_commands[];
extern const SetCommand aaabb_commands[];
extern const SetCommand auto_league_commands[];
extern const SetCommand inf_watt_commands[];
extern const SetCommand pickupberry_commands[];
extern const SetCommand changethedate_commands[];
extern const SetCommand changetheyear_commands[];

// ==========================================
// コマンド配列サイズ
// ==========================================
extern const int mash_a_size;
extern const int aaabb_size;
extern const int auto_league_size;
extern const int inf_watt_size;
extern const int pickupberry_size;
extern const int changethedate_size;
extern const int changetheyear_size;

// ==========================================
// 外部関数宣言
// ==========================================

// ステートマシン実行
void SwitchFunction(void);

// コマンド適用
switch_report_t ApplyButtonCommand(const SetCommand* commands, switch_report_t ReportData);

// コマンド列実行（汎用）
void GetNextReportFromCommands(const SetCommand* commands, const int step_size);
void GetNextReportFromCommandsforChangeTheDate(const SetCommand* commands, const int step_size);
void GetNextReportFromCommandsforChangeTheYear(const SetCommand* commands, const int step_size);

// 互換性のための旧関数宣言
void parse_preset_command(const char* cmd);
void update_preset_state(void);

#endif // PRESETS_H
