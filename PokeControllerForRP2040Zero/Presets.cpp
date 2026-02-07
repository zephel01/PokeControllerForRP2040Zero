/**
 * Presets.cpp - プリセットコマンド実装（Pico互換方式）
 * v1.4.0: プリセットコマンド機能追加
 * v1.5.0: Pico互換コマンドシステム実装、日付変更コマンドを固定プリセット方式に変更
 */

#include "Presets.h"
#include "Common.h"

// ==========================================
// 外部変数（コマンド実行状態管理）
// ==========================================
ProcessState proc_state = PRESET_NONE;

// コマンド実行フラグ
static bool blduration = false;
static bool blwaittime = false;

// コマンドカウンタ
static int cnt_command = 0;

// 前回のレポート（コマンド実行前の状態）
static switch_report_t last_pc_report;

// タイムスタンプ
static unsigned long s_ultime = 0;

// ステップサイズバッファ
static int step_size_buf = INT8_MAX;

// 年変更カウンタ（日付変更用）
static int YearChangeCnt = 0;
static int MonthChangeCnt = 0;
static int DayChangeCnt = 0;

// ==========================================
// ヘルパー関数
// ==========================================

/**
 * レポート送信（gp_reportに反映）
 */
static inline void sendReportOnly(switch_report_t report) {
  // gp_reportを更新（.inoでSwitchに送信される）
  gp_report = report;
}

// ==========================================
// コマンド配列データ
// ==========================================

const SetCommand mash_a_commands[] =
{
  {COMMAND_A, 20, 20}
};

const SetCommand aaabb_commands[] =
{
  {COMMAND_A, 40, 260},
  {COMMAND_A, 40, 260},
  {COMMAND_A, 40, 260},
  {COMMAND_B, 40, 310},
  {COMMAND_B, 40, 310}
};

const SetCommand auto_league_commands[] =
{
  {COMMAND_A, 20, 980},
  {COMMAND_A, 20, 980},
  {COMMAND_A, 20, 980},
  {COMMAND_A, 20, 980},
  {COMMAND_A, 20, 980},
  {COMMAND_A, 20, 980},
  {COMMAND_A, 20, 980},
  {COMMAND_A, 20, 980},
  {COMMAND_A, 20, 980},
  {COMMAND_A, 20, 980},
  {COMMAND_B, 20, 980}
};

const SetCommand inf_watt_commands[] =
{
  {COMMAND_A, 40, 960},
  {COMMAND_B, 40, 1460},
  {COMMAND_B, 40, 1460},
  {COMMAND_B, 40, 1460},
  {COMMAND_B, 40, 1460},
  {COMMAND_B, 40, 1460},
  {COMMAND_A, 40, 1460},
  {COMMAND_A, 40, 3460},
  {COMMAND_HOME, 100, 700},
  {COMMAND_LEFT, 40, 160},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_LEFT, 40, 0},
  {COMMAND_A, 40, 860},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 40},
  {COMMAND_A, 40, 260},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 600, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_A, 40, 210},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 40},
  {COMMAND_A, 40, 210},
  {COMMAND_RIGHT, 40, 0},
  {COMMAND_RS_RIGHT, 40, 0},
  {COMMAND_UP, 40, 0},
  {COMMAND_RIGHT, 40, 0},
  {COMMAND_RS_RIGHT, 40, 0},
  {COMMAND_RIGHT, 40, 40},
  {COMMAND_A, 40, 310},
  {COMMAND_HOME, 100, 700},
  {COMMAND_HOME, 100, 700},
  {COMMAND_B, 40, 740},
  {COMMAND_A, 40, 3440}
};

const SetCommand pickupberry_commands[] =
{
  {COMMAND_L, 40, 40},
  {COMMAND_A, 40, 360},
  {COMMAND_A, 40, 360},
  {COMMAND_A, 40, 360},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_B, 40, 460},
  {COMMAND_HOME, 100, 700},
  {COMMAND_LEFT, 40, 160},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_LEFT, 40, 0},
  {COMMAND_A, 40, 860},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 40},
  {COMMAND_A, 40, 260},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 600, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_A, 40, 210},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 40},
  {COMMAND_A, 40, 210},
  {COMMAND_RIGHT, 40, 0},
  {COMMAND_RS_RIGHT, 40, 0},
  {COMMAND_UP, 40, 0},
  {COMMAND_RIGHT, 40, 0},
  {COMMAND_RS_RIGHT, 40, 0},
  {COMMAND_RIGHT, 40, 40},
  {COMMAND_A, 40, 310},
  {COMMAND_HOME, 100, 700},
  {COMMAND_HOME, 100, 700}
};

const SetCommand changethedate_commands[] =
{
  {COMMAND_NONE, 40, 40},
  {COMMAND_LEFT, 40, 160},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_LEFT, 40, 0},
  {COMMAND_A, 40, 860},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 40},
  {COMMAND_A, 40, 260},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 600, 0},
  {COMMAND_RS_DOWN, 40, 0},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_A, 40, 210},
  {COMMAND_DOWN, 40, 0},
  {COMMAND_RS_DOWN, 40, 40},
  {COMMAND_A, 40, 210},
  {COMMAND_UP, 40, 40},
  {COMMAND_RIGHT, 40, 0},
  {COMMAND_UP, 40, 40},
  {COMMAND_RS_RIGHT, 40, 0},
  {COMMAND_UP, 40, 40},
  {COMMAND_RIGHT, 40, 0},
  {COMMAND_RS_RIGHT, 40, 0},
  {COMMAND_RIGHT, 40, 40},
  {COMMAND_A, 40, 310},
  {COMMAND_HOME, 100, 700},
  {COMMAND_NONE, 100, 700}
};

const SetCommand changetheyear_commands[] =
{
  {COMMAND_NONE, 40, 40},
  {COMMAND_LEFT, 40, 0},
  {COMMAND_RS_LEFT, 40, 0},
  {COMMAND_LEFT, 40, 0},
  {COMMAND_RS_LEFT, 40, 0},
  {COMMAND_LEFT, 40, 0},
  {COMMAND_UP, 40, 0},
  {COMMAND_RIGHT, 40, 0},
  {COMMAND_RS_RIGHT, 40, 0},
  {COMMAND_RIGHT, 40, 0},
  {COMMAND_RS_RIGHT, 40, 0},
  {COMMAND_RIGHT, 40, 40},
  {COMMAND_A, 40, 160},
  {COMMAND_A, 40, 160},
  {COMMAND_LEFT, 40, 0},
  {COMMAND_RS_LEFT, 40, 0},
  {COMMAND_LEFT, 40, 0},
  {COMMAND_RS_LEFT, 40, 0},
  {COMMAND_LEFT, 40, 0},
  {COMMAND_DOWN, 5400, 0},
  {COMMAND_RIGHT, 40, 0},
  {COMMAND_RS_RIGHT, 40, 0},
  {COMMAND_RIGHT, 40, 0},
  {COMMAND_RS_RIGHT, 40, 0},
  {COMMAND_RIGHT, 40, 40},
  {COMMAND_A, 40, 160},
  {COMMAND_A, 40, 160},
  {COMMAND_B, 40, 160},
  {COMMAND_UP, 40, 160},
  {COMMAND_A, 40, 160},
  {COMMAND_NONE, 100, 700}
};

// ==========================================
// コマンド配列サイズ
// ==========================================
const int mash_a_size = (int)(sizeof(mash_a_commands) / sizeof(SetCommand));
const int aaabb_size = (int)(sizeof(aaabb_commands) / sizeof(SetCommand));
const int auto_league_size = (int)(sizeof(auto_league_commands) / sizeof(SetCommand));
const int inf_watt_size = (int)(sizeof(inf_watt_commands) / sizeof(SetCommand));
const int pickupberry_size = (int)(sizeof(pickupberry_commands) / sizeof(SetCommand));
const int changethedate_size = (int)(sizeof(changethedate_commands) / sizeof(SetCommand));
const int changetheyear_size = (int)(sizeof(changetheyear_commands) / sizeof(SetCommand));

// ==========================================
// ApplyButtonCommand - コマンドをレポートに適用
// ==========================================
switch_report_t ApplyButtonCommand(const SetCommand* commands, switch_report_t ReportData)
{
  uint8_t button = commands[cnt_command].command;
  switch (button)
  {
    case COMMAND_UP:
      gp_report.ly = STICK_MIN;
      break;

    case COMMAND_LEFT:
      gp_report.lx = STICK_MIN;
      break;

    case COMMAND_DOWN:
      gp_report.ly = STICK_MAX;
      break;

    case COMMAND_RIGHT:
      gp_report.lx = STICK_MAX;
      break;

    case COMMAND_A:
      gp_report.buttons |= BUTTON_A;
      break;

    case COMMAND_B:
      gp_report.buttons |= BUTTON_B;
      break;

    case COMMAND_X:
      gp_report.buttons |= BUTTON_X;
      break;

    case COMMAND_Y:
      gp_report.buttons |= BUTTON_Y;
      break;

    case COMMAND_L:
      gp_report.buttons |= BUTTON_L;
      break;

    case COMMAND_R:
      gp_report.buttons |= BUTTON_R;
      break;

    case COMMAND_ZL:
      gp_report.buttons |= BUTTON_ZL;
      break;

    case COMMAND_ZR:
      gp_report.buttons |= BUTTON_ZR;
      break;

    case COMMAND_TRIGGERS:
      gp_report.buttons |= BUTTON_L | BUTTON_R;
      break;

    case COMMAND_UPLEFT:
      gp_report.lx = STICK_MIN;
      gp_report.ly = STICK_MIN;
      break;

    case COMMAND_UPRIGHT:
      gp_report.lx = STICK_MAX;
      gp_report.ly = STICK_MIN;
      break;

    case COMMAND_DOWNRIGHT:
      gp_report.lx = STICK_MAX;
      gp_report.ly = STICK_MAX;
      break;

    case COMMAND_DOWNLEFT:
      gp_report.lx = STICK_MIN;
      gp_report.ly = STICK_MAX;
      break;

    case COMMAND_PLUS:
      gp_report.buttons |= BUTTON_PLUS;
      break;

    case COMMAND_MINUS:
      gp_report.buttons |= BUTTON_MINUS;
      break;

    case COMMAND_HOME:
      gp_report.buttons |= BUTTON_HOME;
      break;

    case COMMAND_CAPTURE:
      gp_report.buttons |= BUTTON_CAPTURE;
      break;

    case COMMAND_RS_UP:
      gp_report.ry = STICK_MIN;
      break;

    case COMMAND_RS_LEFT:
      gp_report.rx = STICK_MIN;
      break;

    case COMMAND_RS_DOWN:
      gp_report.ry = STICK_MAX;
      break;

    case COMMAND_RS_RIGHT:
      gp_report.rx = STICK_MAX;
      break;

    case COMMAND_RS_UPLEFT:
      gp_report.rx = STICK_MIN;
      gp_report.ry = STICK_MIN;
      break;

    case COMMAND_RS_UPRIGHT:
      gp_report.rx = STICK_MAX;
      gp_report.ry = STICK_MIN;
      break;

    case COMMAND_RS_DOWNRIGHT:
      gp_report.rx = STICK_MAX;
      gp_report.ry = STICK_MAX;
      break;

    case COMMAND_RS_DOWNLEFT:
      gp_report.rx = STICK_MIN;
      gp_report.ry = STICK_MAX;
      break;

    case COMMAND_HAT_TOP:
      gp_report.hat = HAT_UP;
      break;

    case COMMAND_HAT_TOP_RIGHT:
      gp_report.hat = HAT_UP_RIGHT;
      break;

    case COMMAND_HAT_RIGHT:
      gp_report.hat = HAT_RIGHT;
      break;

    case COMMAND_HAT_BOTTOM_RIGHT:
      gp_report.hat = HAT_DOWN_RIGHT;
      break;

    case COMMAND_HAT_BOTTOM:
      gp_report.hat = HAT_DOWN;
      break;

    case COMMAND_HAT_BOTTOM_LEFT:
      gp_report.hat = HAT_DOWN_LEFT;
      break;

    case COMMAND_HAT_LEFT:
      gp_report.hat = HAT_LEFT;
      break;

    case COMMAND_HAT_TOP_LEFT:
      gp_report.hat = HAT_UP_LEFT;
      break;

    case COMMAND_NONE:
    case COMMAND_NOP:
    default:
      // 何もしない
      break;
  }

  return ReportData;
}

// ==========================================
// GetNextReportFromCommands - コマンド列実行（汎用）
// ==========================================
void GetNextReportFromCommands(const SetCommand* commands, const int step_size)
{
  if ((blduration == false) && (blwaittime == false))
  {
    memcpy(&last_pc_report, &gp_report, sizeof(switch_report_t));
    ApplyButtonCommand(commands, gp_report);
    sendReportOnly(gp_report);
    s_ultime = millis();
    blduration = true;
    blwaittime = true;
    return;
  }
  else if ((blduration == true) && (blwaittime == true))
  {
    if (millis() - s_ultime > (unsigned long)commands[cnt_command].duration)
    {
      sendReportOnly(last_pc_report);
      s_ultime = millis();
      blduration = false;
    }
    return;
  }
  else
  {
    if (millis() - s_ultime > (unsigned long)commands[cnt_command].waittime)
    {
      memcpy(&gp_report, &last_pc_report, sizeof(switch_report_t));
      cnt_command++;
      if (cnt_command >= step_size)
      {
        cnt_command = 0;
      }
      blduration = false;
      blwaittime = false;
    }
  }
}

// ==========================================
// GetNextReportFromCommandsforChangeTheDate - 日付変更コマンド実行
// ==========================================
void GetNextReportFromCommandsforChangeTheDate(const SetCommand* commands, const int step_size)
{
  if ((blduration == false) && (blwaittime == false))
  {
    memcpy(&last_pc_report, &gp_report, sizeof(switch_report_t));
    ApplyButtonCommand(commands, gp_report);
    sendReportOnly(gp_report);
    s_ultime = millis();
    blduration = true;
    blwaittime = true;
    return;
  }
  else if ((blduration == true) && (blwaittime == true))
  {
    if (millis() - s_ultime > (unsigned long)commands[cnt_command].duration)
    {
      sendReportOnly(last_pc_report);
      s_ultime = millis();
      blduration = false;
    }
    return;
  }
  else
  {
    if (millis() - s_ultime > (unsigned long)commands[cnt_command].waittime)
    {
      memcpy(&gp_report, &last_pc_report, sizeof(switch_report_t));
      cnt_command++;
      if(cnt_command >= step_size)
      {
        cnt_command = step_size - 1;
      }
      if((cnt_command == INDEX_ARRAY_YEAR + 1) && (YearChangeCnt > 1))
      {
        YearChangeCnt--;
        cnt_command = INDEX_ARRAY_YEAR;
      }
      if(cnt_command == INDEX_ARRAY_YEAR)
      {
        YearChangeCnt++;
        cnt_command++;
      }
      if((cnt_command == INDEX_ARRAY_MONTH + 1) && (MonthChangeCnt > 1))
      {
        MonthChangeCnt--;
        cnt_command = INDEX_ARRAY_MONTH;
      }
      if(cnt_command == INDEX_ARRAY_MONTH)
      {
        MonthChangeCnt++;
        cnt_command++;
      }
      if((cnt_command == INDEX_ARRAY_DAY + 1) && (DayChangeCnt > 1))
      {
        DayChangeCnt--;
        cnt_command = INDEX_ARRAY_DAY;
      }
      if(cnt_command == INDEX_ARRAY_DAY)
      {
        DayChangeCnt++;
        cnt_command++;
      }
      blduration = false;
      blwaittime = false;
    }
  }
}

// ==========================================
// GetNextReportFromCommandsforChangeTheYear - 年変更コマンド実行
// ==========================================
void GetNextReportFromCommandsforChangeTheYear(const SetCommand* commands, const int step_size)
{
  if ((blduration == false) && (blwaittime == false))
  {
    memcpy(&last_pc_report, &gp_report, sizeof(switch_report_t));
    ApplyButtonCommand(commands, gp_report);
    sendReportOnly(gp_report);
    s_ultime = millis();
    blduration = true;
    blwaittime = true;
    return;
  }
  else if ((blduration == true) && (blwaittime == true))
  {
    if (millis() - s_ultime > (unsigned long)commands[cnt_command].duration)
    {
      sendReportOnly(last_pc_report);
      s_ultime = millis();
      blduration = false;
    }
    return;
  }
  else
  {
    if (millis() - s_ultime > (unsigned long)commands[cnt_command].waittime)
    {
      memcpy(&gp_report, &last_pc_report, sizeof(switch_report_t));
      cnt_command++;
      if ((YearChangeCnt > 0) && (cnt_command == 14))
      {
        if (step_size_buf == INT8_MAX)
        {
          step_size_buf = cnt_command;
        }
        cnt_command = 27;
      }
      else if ((YearChangeCnt > 0) && (cnt_command == 27))
      {
        YearChangeCnt--;
        if (YearChangeCnt > 0)
        {
          cnt_command = 1;
        }
        else
        {
          cnt_command++;
        }
      }
      else if ((YearChangeCnt == 0) && (cnt_command > step_size_buf - 1))
      {
        cnt_command = step_size_buf - 1;
      }
      else if ((YearChangeCnt == 0) && (cnt_command == step_size_buf - 1))
      {
        proc_state = PRESET_NONE;
        cnt_command = 0;
        step_size_buf = INT8_MAX;
      }
      blduration = false;
      blwaittime = false;
    }
  }
}

// ==========================================
// SwitchFunction - ステートマシン実行
// ==========================================
void SwitchFunction(void)
{
  switch (proc_state)
  {
    case PRESET_NONE:
    case PC_CALL:
      sendReportOnly(gp_report);
      break;

    case PC_CALL_STRING:
      // TODO: キーボード文字列入力実装
      break;

    case PC_CALL_KEYBOARD:
    case PC_CALL_KEYBOARD_PRESS:
    case PC_CALL_KEYBOARD_RELEASE:
      // TODO: キーボード特殊キー実装
      break;

    case MASH_A:
      GetNextReportFromCommands(&mash_a_commands[0], mash_a_size);
      break;

    case AAABB:
      GetNextReportFromCommands(&aaabb_commands[0], aaabb_size);
      break;

    case AUTO_LEAGUE:
      gp_report.lx = 172;
      gp_report.ly = 7;
      GetNextReportFromCommands(&auto_league_commands[0], auto_league_size);
      break;

    case INF_WATT:
      GetNextReportFromCommands(&inf_watt_commands[0], inf_watt_size);
      break;

    case PICKUPBERRY:
      GetNextReportFromCommands(&pickupberry_commands[0], pickupberry_size);
      break;

    case CHANGETHEDATE:
      GetNextReportFromCommandsforChangeTheDate(&changethedate_commands[0], changethedate_size);
      break;

    case CHANGETHEYEAR:
      GetNextReportFromCommandsforChangeTheYear(&changetheyear_commands[0], changetheyear_size);
      break;

    default:
      break;
  }
}

// ==========================================
// 互換性のための旧関数実装
// ==========================================

void parse_preset_command(const char* cmd) {
  if (strcmp(cmd, "mash_a") == 0) {
    proc_state = MASH_A;
    cnt_command = 0;
    blduration = false;
    blwaittime = false;
  }
  else if (strcmp(cmd, "aaabb") == 0) {
    proc_state = AAABB;
    cnt_command = 0;
    blduration = false;
    blwaittime = false;
  }
  else if (strcmp(cmd, "auto_league") == 0) {
    proc_state = AUTO_LEAGUE;
    cnt_command = 0;
    blduration = false;
    blwaittime = false;
  }
  else if (strcmp(cmd, "inf_watt") == 0) {
    proc_state = INF_WATT;
    cnt_command = 0;
    blduration = false;
    blwaittime = false;
  }
  else if (strcmp(cmd, "pickupberry") == 0) {
    proc_state = PICKUPBERRY;
    cnt_command = 0;
    blduration = false;
    blwaittime = false;
  }
  else if (strcmp(cmd, "changethedate") == 0) {
    proc_state = CHANGETHEDATE;
    cnt_command = 0;
    blduration = false;
    blwaittime = false;
    YearChangeCnt = 0;
    MonthChangeCnt = 0;
    DayChangeCnt = 0;
    step_size_buf = INT8_MAX;
  }
  else if (strcmp(cmd, "changetheyear") == 0) {
    proc_state = CHANGETHEYEAR;
    cnt_command = 0;
    blduration = false;
    blwaittime = false;
    YearChangeCnt = 0;
    step_size_buf = INT8_MAX;
  }
}

void update_preset_state(void) {
  SwitchFunction();
}
