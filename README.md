# PokeControllerForRP2040Zero

RP2040-Zero (Waveshare) を使って Nintendo Switch を操作するためのプロジェクトです。

## 特徴

- **Waveshare RP2040-Zero に最適化**: 小型なボードで Switch の有線コントローラーとして動作します。
- **Poke-Controller Modified 対応**: シリアル通信プロトコルを解釈し、PC からの操作を受け付けます。
- **Adafruit TinyUSB Stack 使用**: 安定した USB HID 動作を提供します。

## 開発環境

- **Arduino IDE**
- **Arduino-Pico コア** (Earle Philhower 版)
- **Adafruit TinyUSB Library** (ライブラリマネージャからインストール)

### ボード設定

- ボード: `Waveshare RP2040 Zero`
- USB Stack: `Adafruit TinyUSB`

## 配線 (Wiring)

| RP2040-Zero | 機能     | 接続先                  |
| :---------- | :------- | :---------------------- |
| **GP0**     | UART0 TX | USB-UART 変換の **RX**  |
| **GP1**     | UART0 RX | USB-UART 変換の **TX**  |
| **GND**     | GND      | USB-UART 変換の **GND** |

USB-C ポートは Switch 本体に接続してください。

## ライセンス

[MIT License](LICENSE)
