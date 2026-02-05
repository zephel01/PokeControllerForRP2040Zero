/**
 * Presets.h - プリセットコマンド定義
 * v1.4.0: プリセットコマンド機能追加
 */

#ifndef PRESETS_H
#define PRESETS_H

#include <Arduino.h>

// プリセットコマンド列挙型
typedef enum {
  PRESET_NONE = 0,
  PRESET_MASH_A,        // Aボタン連打
  PRESET_AAABB,         // AAABBパターン
  PRESET_AUTO_LEAGUE,    // 自動リーグ戦闘
  PRESET_INF_WATT,      // 無限ワット収集
  PRESET_PICKUPBERRY,    // ベリー収集
} PresetCommand;

// プリセットコマンドステート
typedef struct {
  PresetCommand command;
  int step;
  uint32_t last_step_time;
  bool is_running;
} PresetState;

// タイミング定数（ms）
#define MASH_A_DURATION          20
#define MASH_A_WAIT              20
#define AAABB_DURATION           20
#define AAABB_WAIT               20
#define BUTTON_SHORT_PRESS       50
#define BUTTON_LONG_PRESS        500
#define MENU_NAV_DELAY           200
#define ANIMATION_WAIT           1000
#define BATTLE_WAIT              3000

// 外部関数宣言
void parse_preset_command(const char* cmd);
void update_preset_state(void);
void run_preset_mash_a(void);
void run_preset_aaabb(void);
void run_preset_auto_league(void);
void run_preset_inf_watt(void);
void run_preset_pickupberry(void);

#endif // PRESETS_H
