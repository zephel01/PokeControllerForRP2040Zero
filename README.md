# PokeControllerForRP2040Zero

RP2040-Zero (Waveshare) を使って Nintendo Switch を操作するためのプロジェクトです。
PC上の「Poke-Controller Modified」等のツールから UART 経由でコマンドを受け取り、Switch の有線コントローラー (HORIPAD) として動作します。

## 特徴

- **Waveshare RP2040-Zero に最適化**: 小型かつ安価なボードで動作します。
- **Poke-Controller Modified 対応**: 標準的なシリアルプロトコルを解釈し、PC からの入力を中継します。
- **Adafruit TinyUSB Stack 使用**: 高度なカスタマイズが可能な USB スタックを使用し、Switch との高精度な通信を実現します。

## 必要なもの

### ハードウェア
- **Waveshare RP2040-Zero**
- **USB-UART 変換アダプタ** (PCとの通信用)
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
   - [Releases ページ](https://github.com/zephel01/PokeControllerForRP2040Zero/releases/tag/v1.0.0) から `PokeControllerForRP2040Zero_RP2040Zero_v1.0.0.uf2` をダウンロードします。
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

PC からのシリアル入力を受け取るために、USB-UART 変換アダプタと以下のように接続します。

| RP2040-Zero | 機能     | 接続先 (USB-UART)  |
| :---------- | :------- | :----------------- |
| **GP0**     | UART0 TX | アダプタの **RX**  |
| **GP1**     | UART0 RX | アダプタの **TX**  |
| **GND**     | GND      | アダプタの **GND** |

- **USB-C ポート**: Nintendo Switch のドック（または本体）に接続します。
- **UART 入力**: PC から 115200bps でコマンドを送信します。

---

## プロトコル

Poke-Controller Modified の標準プロトコル（16進文字列）をサポートしています。
例: `0004 08 80 80 80 80\n`

---

## LED ステータス

RP2040-Zero 上の RGB LED で現在の状態を確認できます。

| 色             | 状態       | 詳細                                                            |
| :------------- | :--------- | :-------------------------------------------------------------- |
| **赤 (Red)**   | **未接続** | PC または Switch と正しく USB 通信ができていません。            |
| **青 (Blue)**  | **待機中** | USB 接続は確立していますが、PC からのコマンド受信がありません。 |
| **緑 (Green)** | **動作中** | PC からコマンドを受信し、コントローラー操作を実行中です。       |

---

## ライセンス

[MIT License](LICENSE)

---

## 謝辞・参考プロジェクト

本プロジェクトは、以下の素晴らしいオープンソースプロジェクトや情報を参考に作成されました。

- **[Adafruit TinyUSB Library](https://github.com/adafruit/Adafruit_TinyUSB_Arduino)**: RP2040 での柔軟な USB デバイス実装を可能にするライブラリ
- **[sorasen2020/PokeControllerForPico](https://github.com/sorasen2020/PokeControllerForPico)**: RP2040 (Raspberry Pi Pico) を用いた実装の先駆的なプロジェクト
- **[Earle Philhower's Arduino-Pico Core](https://github.com/earlephilhower/arduino-pico)**: Arduino 環境で RP2040 を扱うための強力なコア
- **[HORI PAD](https://hori.jp/)**: コントローラーとしての動作プロファイルの参考
