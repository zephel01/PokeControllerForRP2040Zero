/**
 * Presets.cpp - プリセットコマンド実装
 */

#include "Presets.h"
#include "Common.h"

// プリセット状態管理
static PresetState preset_state = {PRESET_NONE, 0, 0, false};

// プリセットコマンド解析
void parse_preset_command(const char* cmd) {
  Serial.printf("Preset Command: %s\n", cmd);
  
  if (strcmp(cmd, "mash_a") == 0) {
    preset_state.command = PRESET_MASH_A;
    preset_state.step = 0;
    preset_state.is_running = true;
    preset_state.last_step_time = millis();
  }
  else if (strcmp(cmd, "aaabb") == 0) {
    preset_state.command = PRESET_AAABB;
    preset_state.step = 0;
    preset_state.is_running = true;
    preset_state.last_step_time = millis();
  }
  else if (strcmp(cmd, "auto_league") == 0) {
    preset_state.command = PRESET_AUTO_LEAGUE;
    preset_state.step = 0;
    preset_state.is_running = true;
    preset_state.last_step_time = millis();
  }
  else if (strcmp(cmd, "inf_watt") == 0) {
    preset_state.command = PRESET_INF_WATT;
    preset_state.step = 0;
    preset_state.is_running = true;
    preset_state.last_step_time = millis();
  }
  else if (strcmp(cmd, "pickupberry") == 0) {
    preset_state.command = PRESET_PICKUPBERRY;
    preset_state.step = 0;
    preset_state.is_running = true;
    preset_state.last_step_time = millis();
  }
}

// プリセット状態更新（loop内から呼び出し）
void update_preset_state(void) {
  if (!preset_state.is_running) return;
  
  uint32_t now = millis();
  
  switch (preset_state.command) {
    case PRESET_MASH_A:
      run_preset_mash_a();
      break;
    case PRESET_AAABB:
      run_preset_aaabb();
      break;
    case PRESET_AUTO_LEAGUE:
      run_preset_auto_league();
      break;
    case PRESET_INF_WATT:
      run_preset_inf_watt();
      break;
    case PRESET_PICKUPBERRY:
      run_preset_pickupberry();
      break;
    default:
      preset_state.is_running = false;
      break;
  }
}

// ==================== プリセット実装 ====================

// Aボタン連打（終了条件なし、'end'で停止）
void run_preset_mash_a(void) {
  uint32_t now = millis();
  if (now - preset_state.last_step_time < (MASH_A_DURATION + MASH_A_WAIT)) return;
  
  press_button(BUTTON_A, MASH_A_DURATION);
  preset_state.last_step_time = now;
}

// AAABBパターン（Aを3回、Bを2回押す）
void run_preset_aaabb(void) {
  uint32_t now = millis();
  if (now - preset_state.last_step_time < (AAABB_DURATION + AAABB_WAIT)) return;
  
  switch (preset_state.step) {
    case 0: case 1: case 2:
      press_button(BUTTON_A, AAABB_DURATION);
      break;
    case 3: case 4:
      press_button(BUTTON_B, AAABB_DURATION);
      break;
    default:
      preset_state.is_running = false;
      Serial.println("Preset: aaabb completed");
      return;
  }
  preset_state.step++;
  preset_state.last_step_time = now;
}

// 自動リーグ戦闘（簡易実装）
void run_preset_auto_league(void) {
  uint32_t now = millis();
  
  // ステートマシン
  // 0-1: 対戦開始待ち（A押下）
  // 2-4: 戦闘中（A連打）
  // 5: 対戦終了待ち
  // 6: 次の対戦へ（A押下）
  
  switch (preset_state.step) {
    case 0:
      press_button(BUTTON_A, BUTTON_SHORT_PRESS);
      preset_state.step = 1;
      preset_state.last_step_time = now;
      break;
      
    case 1:
      if (now - preset_state.last_step_time > ANIMATION_WAIT) {
        preset_state.step = 2;
      }
      break;
      
    case 2: // 戦闘中（A連打）
      if (now - preset_state.last_step_time > BATTLE_WAIT) {
        preset_state.step = 5;
      } else {
        press_button(BUTTON_A, MASH_A_DURATION);
        preset_state.last_step_time = now;
      }
      break;
      
    case 5:
      press_button(BUTTON_A, BUTTON_SHORT_PRESS);
      preset_state.step = 6;
      preset_state.last_step_time = now;
      break;
      
    case 6:
      if (now - preset_state.last_step_time > ANIMATION_WAIT) {
        preset_state.step = 0; // ループ
      }
      break;
  }
}

// 無限ワット収集（日付変更含む）
void run_preset_inf_watt(void) {
  uint32_t now = millis();
  
  switch (preset_state.step) {
    case 0: // HOMEを押して設定画面へ
      press_button(BUTTON_HOME, BUTTON_SHORT_PRESS);
      preset_state.step = 1;
      preset_state.last_step_time = now;
      break;
      
    case 1:
      if (now - preset_state.last_step_time > MENU_NAV_DELAY) {
        press_hat(HAT_RIGHT, MENU_NAV_DELAY); // 右へ
        preset_state.step = 2;
        preset_state.last_step_time = now;
      }
      break;
      
    case 2:
      if (now - preset_state.last_step_time > MENU_NAV_DELAY) {
        press_hat(HAT_DOWN, MENU_NAV_DELAY); // 下へ
        preset_state.step = 3;
        preset_state.last_step_time = now;
      }
      break;
      
    case 3:
      if (now - preset_state.last_step_time > MENU_NAV_DELAY) {
        press_button(BUTTON_A, BUTTON_SHORT_PRESS); // 設定選択
        preset_state.step = 4;
        preset_state.last_step_time = now;
      }
      break;
      
    case 4:
      if (now - preset_state.last_step_time > MENU_NAV_DELAY) {
        press_hat(HAT_DOWN, MENU_NAV_DELAY); // 下へ（日付変更へ）
        preset_state.step = 5;
        preset_state.last_step_time = now;
      }
      break;
      
    case 5:
      if (now - preset_state.last_step_time > MENU_NAV_DELAY) {
        press_button(BUTTON_A, BUTTON_SHORT_PRESS); // 日付変更選択
        preset_state.step = 6;
        preset_state.last_step_time = now;
      }
      break;
      
    case 6:
      if (now - preset_state.last_step_time > MENU_NAV_DELAY) {
        press_hat(HAT_RIGHT, MENU_NAV_DELAY); // 右へ（日付進める）
        preset_state.step = 7;
        preset_state.last_step_time = now;
      }
      break;
      
    case 7:
      if (now - preset_state.last_step_time > MENU_NAV_DELAY) {
        press_button(BUTTON_A, BUTTON_SHORT_PRESS); // 確定
        preset_state.step = 8;
        preset_state.last_step_time = now;
      }
      break;
      
    case 8:
      if (now - preset_state.last_step_time > ANIMATION_WAIT) {
        press_button(BUTTON_HOME, BUTTON_SHORT_PRESS); // HOMEに戻る
        preset_state.step = 0; // ループ
      }
      break;
  }
}

// ベリー収集
void run_preset_pickupberry(void) {
  uint32_t now = millis();
  
  switch (preset_state.step) {
    case 0: // Aでベリーを拾う
      press_button(BUTTON_A, BUTTON_SHORT_PRESS);
      preset_state.step = 1;
      preset_state.last_step_time = now;
      break;
      
    case 1:
      if (now - preset_state.last_step_time > ANIMATION_WAIT) {
        preset_state.step = 0; // ループ
      }
      break;
  }
}
