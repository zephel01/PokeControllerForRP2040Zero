# PokeControllerForRP2040Zero

RP2040-Zero (Waveshare) を使って Nintendo Switch を操作するためのプロジェクトです。
PC上の「Poke-Controller Modified」等のツールから UART 経由でコマンドを受け取り、Switch の有線コントローラー (HORIPAD) として動作します。

> **Current Version: v1.4.3**

> PokeControllerForPico互換コマンドシステムを実装。SetCommand構造体、BUTTON_DEFINE列挙型、SwitchFunction()、ApplyButtonCommand()、GetNextReportFromCommands()系のユーティリティ関数を追加。全プリセットコマンドに対応。

## 特徴

- **Waveshare RP2040-Zero に最適化**: 小型かつ安価なボードで動作します。
- **Poke-Controller Modified 対応**: 標準的なシリアルプロトコルを解釈し、PC からの入力を中継します。
- **高信頼性・堅牢設計 (v1.4.3 強化)**:
- **Pico互換コマンドシステム (v1.4.3)**: PokeControllerForPicoと互換なコマンドシステムを実装。SetCommand構造体、BUTTON_DEFINE列挙型、SwitchFunction()、ApplyButtonCommand()、GetNextReportFromCommands()系のユーティリティ関数を追加。
- **プリセットコマンド (v1.4.0)**: `mash_a` (連打), `aaabb` (AAABBパターン), `auto_league` (自動リーグ戦闘), `inf_watt` (無限ワット), `pickupberry` (ベリー収集) などの定型操作を実装。
- **日付・年変更 (v1.4.0)**: 本体の時計を自動で進める/戻すコマンドを実装。
- **JISキーボード対応 (v1.4.0)**: 日本語キーボード環境でも記号がズレない正確な入力を実現。
- **高レベルAPI (v1.4.0)**: スティック操作・ボタン操作の柔軟なAPIを追加。
- **WDT (Watchdog Timer)**: 万が一のフリーズ時も自動で再起動します。10秒に延長し安定性を向上。
- **UART 自動同期回復**: バッファ溢れやパケット破損時に自動で通信同期を再確立します。
- **自動リカバリ**: USB 切断やスリープ復帰を検知し、自動的に再接続処理を行います。
- **Activity LED**: 受信時に緑LEDが一瞬点滅し、通信状態を視覚的に確認可能。
- **デバッグ機能**: USB CDC (シリアルポート) 経由での動作ログ出力に対応。

## 必要なもの

### ハードウェア
- **Waveshare RP2040-Zero**
- **USB to TTL Serial Adapter** (USB-UART 変換アダプタ、PCとの通信用)
- **USB-C ケーブル** (Switchとの接続用)
- ジャンパ線など

### ソフトウェア / ライブラリ
- **Arduino IDE** (2.0以降推奨)
- **Arduino-Pico コア** (Earle Philhower版): RP2040 を Arduino IDE で扱うために必要です。
- **Adafruit TinyUSB Library**: USB HID 通信に使用します。

---

## インストール方法 (簡単)

ビルド済みのファームウェア (`.uf2` ファイル) を使用すると、Arduino IDE を使わずに簡単にセットアップできます。

1. **RP2040-Zero を PC に接続する**:
    - ボード上の **BOOT ボタン** を押しながら USB ケーブルを PC に接続します。
    - PC に「RPI-RP2」という名前のドライブとして認識されます。

2. **ファームウェアを書き込む**:
    - [Releases ページ](https://github.com/zephel01/PokeControllerForRP2040Zero/releases) から最新の `.uf2` ファイル (例: `PokeControllerForRP2040Zero_RP2040Zero_v1.4.3.uf2`) をダウンロードします。
    - ダウンロードした `.uf2` ファイルを、先ほどの「RPI-RP2」ドライブにドラッグ＆ドロップします。

3. **完了**:
    - 書き込みが完了すると、RP2040-Zero が自動的に再起動し、コントローラーとして動作を開始します。

※ 自分でコードを修正したりビルドしたい場合は、以下の「開発環境のセットアップ」に進んでください。

---

## 開発環境のセットアップ (ビルドする場合)

### 1. Arduino IDE の準備

1. Arduino IDE を起動し、`基本設定 (Preferences)` を開きます。
2. `追加のボードマネージャのURL` に以下の URL を追加します。
    ```
    https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
    ```
3. `ツール` -> `ボード` -> `ボードマネージャ` を開き、**"Raspberry Pi Pico/RP2040"** を検索してインストールします。

### 2. ライブラリのインストール

1. `ツール` -> `ライブラリを管理...` を開きます。
2. **"Adafruit TinyUSB Library"** を検索してインストールします。

### 3. 書き込み設定 (重要)

Arduino IDE でスケッチを開き、以下の設定を行ってから書き込みを行ってください。

- **ボード**: `Waveshare RP2040 Zero`
- **USB Stack**: `Adafruit TinyUSB` (必須)
- **シリアルポート**: RP2040-Zero が接続されているポートを選択

> [!CAUTION]
> **USB Stack を "Adafruit TinyUSB" に変更してください。** デフォルトの "Arduino" スタックでは正常に動作しません。

---

## 配線図 (Wiring)

PC からのシリアル入力を受け取るために、**USB to TTL Serial Adapter** (USB-UART 変換アダプタ) と以下のように接続します。

| RP2040-Zero | 機能     | 接続先 (USB to TTL Adapter) |
| :---------- | :------- | :-------------------------- |
| **GP0**     | UART0 TX | アダプタの **RX**           |
| **GP1**     | UART0 RX | アダプタの **TX**           |
| **GND**     | GND      | アダプタの **GND**          |

- **USB-C ポート**: Nintendo Switch のドック（または本体）に接続します。
- **UART 入力**: PC から 115200bps でコマンドを送信します。

> [!WARNING]
> **電圧と接続に関する重要な注意点**
> 1. **ロジック電圧は 3.3V 必須です**: RP2040 は 5V 耐性がありません。USB-UART 変換アダプタの電圧（TTLレベル）は必ず **3.3V** に設定してください。5V を印加すると RP2040-Zero が破損します。
> 2. **GND を共通にする**: アダプタの GND と RP2040-Zero の GND は必ず接続してください。
> 3. **VCC ピンは接続しない**: RP2040-Zero の電源は Switch の USB ポートから供給されるため、変換アダプタの VCC ピンを繋ぐ必要はありません。複数の電源（Switchとアダプタ）を同時に繋ぐと逆流による故障の原因となるため、**TX, RX, GND の 3 本のみ**で接続することを強く推奨します。

---

## プロトコル

Poke-Controller Modified の標準プロトコル（16進文字列）をサポートしています。
例: `0004 08 80 80 80 80\n`

---

## LED ステータス

RP2040-Zero 上の RGB LED で現在の状態を確認できます。

| 色             | 状態       | 詳細                                                                       |
| :------------- | :--------- | :------------------------------------------------------------------------- |
| **赤 (Red)**   | **未接続** | PC または Switch と正しく USB 通信ができていません。                       |
| **青 (Blue)**  | **待機中** | USB 接続は確立していますが、コマンド受信がありません（減光点灯）。         |
| **緑 (Green)** | **通信中** | 受信時に一瞬点灯します。連続受信時は高速に点滅しアクティビティを示します。 |
| **赤点滅**     | **エラー** | 受信バッファ溢れなどの異常を検知しました。自動復帰します。                 |

---

## デバッグ機能 (USB CDC)

本ファームウェアは、Switch のコントローラーとして動作しながら、RP2040-Zero 本体の USB ポート経由で **デバッグ用ログ** を出力します。

> [!NOTE]
> デバッグログを確認できるのは、**RP2040-Zero を PC に USB 接続している場合** に限られます。
> Switch ドックに接続している間は、PC からこのログを見ることはできません。

### 接続構成とポートの違い (v1.3.3 以降)

v1.3.3 以降から、**USBケーブル 1本で「操作」と「デバッグ」の両方が可能** になりました。

1.  **USB シリアル (CDC)** (推奨)
    - RP2040-Zero と PC を繋いでいる USB ケーブル経由で通信します。
    - Poke-Controller からの操作コマンド受信と、デバッグログ送信を同時に行います。
    - ボーレートは **115200 bps** を推奨しますが、CDC接続のため設定値に関わらず高速に通信できます。

2.  **物理 UART (TX/RX)** (従来互換)
    - USB-UART アダプタ等を `TX(0)` / `RX(1)` ピンに繋いで操作することも可能です。

---

## USBキーボード機能 (v1.4.0 日本語JIS対応)

本バージョンより、コントローラー入力だけでなく **日本語配列 (JIS) 対応のUSBキーボード入力** に対応しました。
Switch のニックネーム入力画面や、チャット画面などで文字を自動入力できます。
従来の英語配列ベースでの記号のズレ（`@` や `:` など）が解消され、PC上の文字通りに入力されます。

### コマンド仕様

| コマンド  | 引数   | 説明                                 | 例               |
| :-------- | :----- | :----------------------------------- | :--------------- |
| `"`       | 文字列 | 指定した文字列を高速に入力します。   | `"Hello`         |
| `Key`     | Hex    | 指定したキーを 1回押して離します。   | `Key 28` (Enter) |
| `Press`   | Hex    | 指定したキーを押しっぱなしにします。 | `Press 04` (A)   |
| `Release` | (なし) | 押されている全てのキーを離します。   | `Release`        |

> ※ Hex は HID Usage ID (16進数) です。例: `04`=`a`, `05`=`b`, `28`=`Enter`

### [重要] 日本語入力モードの対策

Switch のキーボード画面が「日本語入力（ローマ字入力）」になっていると、英語コマンドを送っても正しく入力されない場合があります。
これを回避するには、文字入力の前に **入力モード切替キー** (全角/半角キー等) を送る必要があります。

**修正済みの Keys.py を使用してください**
本リポジトリ同梱の `Python_Extensions` フォルダにある修正ファイルを使用すると、この対策が自動的に行われます。
詳細は [Python_Extensions/README.md](Python_Extensions/README.md) を参照してください。

### [Tips] Python 側での便利な呼び出し方

上記 `Python_Extensions` を導入すると、スクリプトから以下のように簡単に呼び出せるようになります。

```python
self.keys.type_string("Hello")
```

導入手順と詳細は、[Python_Extensions/README.md](Python_Extensions/README.md) を参照してください。

---

## Pico互換コマンドシステム (v1.4.3)

本バージョンより、PokeControllerForPicoと互換なコマンドシステムを実装しました。

### BUTTON_DEFINE 列挙型
左スティック、右スティック、ボタン、HATスイッチの操作を定義します。

### SetCommand 構造体
コマンド、操作時間、待ち時間を指定します。

### プリセットコマンド
- `mash_a`: Aボタン連打
- `aaabb`: AAABBパターン（A3回、B2回）
- `auto_league`: 自動リーグ戦闘
- `inf_watt`: 無限ワット収集
- `pickupberry`: ベリー収集
- `changethedate`: 日付変更
- `changetheyear`: 年変更

### ユーティリティ関数
- `SwitchFunction()`: ステートマシン実行
- `ApplyButtonCommand()`: コマンドをレポートに適用
- `GetNextReportFromCommands()`: コマンド列実行（汎用）
- `GetNextReportFromCommandsforChangeTheDate()`: 日付変更コマンド実行
- `GetNextReportFromCommandsforChangeTheYear()`: 年変更コマンド実行

---

## 高レベルAPI (v1.4.0)

スティック操作やボタン操作を簡単に行うための高レベルAPIです。

### 主な関数
- `pushButton()`: ボタン押下
- `pushButton2()`: ボタン押下（時間指定）
- `pushHatButton()`: HATボタン押下
- `tiltJoystick()`: スティック傾き
- `useLeftStick()`: 左スティック方向
- `useRightStick()`: 右スティック方向
- `tiltLeftStick()`: 左スティック角度指定

---

## HATスイッチ

方向キーとしての操作をサポートします。

| 値   | 方向 |
| ---- | ---- |
| 0x00 | 上 |
| 0x01 | 右上 |
| 0x02 | 右 |
| 0x03 | 右下 |
| 0x04 | 下 |
| 0x05 | 左下 |
| 0x06 | 左 |
| 0x07 | 左上 |
| 0x08 | 中央 |

---

## プリセット・特殊機能 (v1.4.0)

複雑な操作や定型操作を簡単に行うためのコマンドが追加されました。

### プリセットコマンド
コマンド単体で特定の動作を開始します（小文字で送信してください）。
- `mash_a`: Aボタン連打を開始。
- `aaabb`: AAABBパターン（A3回、B2回）を実行。
- `auto_league`: トーナメント自動周回。
- `inf_watt`: 無限ワット収集（要：日付変更バグが利用可能な場所）。
- `pickupberry`: 木を揺らしてベリーを回収。
- `changethedate`: 日付変更。
- `changetheyear`: 年変更。

### 日付・年変更
本体の設定画面で日付を操作するコマンドです。
- `Date Y/M/D`: 年/月/日の変化量を指定（例: `Date 1/0/1` で1年進める、`Date -1/0/0` で1年戻す）。
- `Year N`: 年の変化量を指定（例: `Year 5` で5年進める、`Year -3` で3年戻す）。

---

## ドキュメント

本プロジェクトの詳細な解説や、安定化設計については以下の記事を参照してください。

- **[📘️ PokeControllerForRP2040Zero 完全解説【決定版】](https://note.com/zephel01/n/ne5a9fec85f12)**
  - アーキテクチャ、TinyUSB の設定、プロトコル解析ロジックなど、仕組み全体の詳細な解説です。

- **[RP2040-Zero で作る Nintendo Switch 自動操作コントローラー【v1.3.0 完全版】](https://note.com/zephel01/n/n2c64da9ea81b)**
  - 安定性・信頼性を向上させた v1.3.0 の実装解説（WDT、安全装置、LEDステータスなど）です。

---

## ライセンス

[MIT License](LICENSE)

---

## 更新履歴 (Changelog)

- **v1.4.3**: PokeControllerForPico互換コマンドシステムを実装。SetCommand構造体、BUTTON_DEFINE列挙型、SwitchFunction()、ApplyButtonCommand()、GetNextReportFromCommands()系のユーティリティ関数を追加。全プリセットコマンドに対応。
- **v1.4.2**: コード整理。タイミング定数の `Common.h` への一元化。
- **v1.4.1**: データ構造の最適化。ボタンデータを 16bit 化し、他のマイコン版との仕様互換性を確保。
- **v1.4.0**: メジャーアップデート。プリセットコマンド、日付・年変更機能、JISキーボード対応、WDT強化、モジュール分割を実施。

---

## 謝辞・参考プロジェクト

本プロジェクトは、以下の素晴らしいオープンソースプロジェクトや情報を参考に作成されました。

- **[Adafruit TinyUSB Library](https://github.com/adafruit/Adafruit_TinyUSB_Arduino)**: RP2040 での柔軟な USB デバイス実装を可能にするライブラリ
- **[sorasen2020/PokeControllerForPico](https://github.com/sorasen2020/PokeControllerForPico)**: RP2040 (Raspberry Pi Pico) を用いた実装の先駆的なプロジェクト
- **[Earle Philhower's Arduino-Pico Core](https://github.com/earlephilhower/arduino-pico)**: Arduino 環境で RP2040 を扱うための強力なコア
- **[HORI PAD](https://hori.jp/)**: コントローラーとしての動作プロファイルの参考
