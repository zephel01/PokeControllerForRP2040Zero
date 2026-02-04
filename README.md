# PokeControllerForRP2040Zero

RP2040-Zero (Waveshare) を使って Nintendo Switch を操作するためのプロジェクトです。
PC上の「Poke-Controller Modified」等のツールから UART 経由でコマンドを受け取り、Switch の有線コントローラー (HORIPAD) として動作します。

## 特徴

- **Waveshare RP2040-Zero に最適化**: 小型かつ安価なボードで動作します。
- **Poke-Controller Modified 対応**: 標準的なシリアルプロトコルを解釈し、PC からの入力を中継します。
- **高信頼性・堅牢設計**:
  - **WDT (Watchdog Timer)**: 万が一のフリーズ時も自動で再起動します。
  - **自動リカバリ**: USB 切断やスリープ復帰を検知し、自動的に再接続処理を行います。
  - **バッファリング**: 高速なコマンド受信でも取りこぼしにくいリングバッファ実装。
- **デバッグ機能**: USB CDC (シリアルポート) 経由での動作ログ出力に対応。

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
   - [Releases ページ](https://github.com/zephel01/PokeControllerForRP2040Zero/releases/tag/v1.3.3) から `PokeControllerForRP2040Zero_RP2040Zero_v1.3.3.uf2` をダウンロードします。
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
| **赤点滅**     | **エラー** | 受信バッファ溢れなどの異常を検知しました。自動復帰します。      |

---

## デバッグ機能 (USB CDC)

本ファームウェアは、Switch のコントローラーとして動作しながら、RP2040-Zero 本体の USB ポート経由で **デバッグ用ログ** を出力します。

> [!NOTE]
> デバッグログを確認できるのは、**RP2040-Zero を PC に USB 接続している場合** に限られます。
> Switch ドックに接続している間は、PC からこのログを見ることはできません。

### 接続構成とポートの違い (v1.3.3 以降)

v1.3.3 から、**USBケーブル 1本で「操作」と「デバッグ」の両方が可能** になりました。

1.  **USB シリアル (CDC)** (推奨)
    *   RP2040-Zero と PC を繋いでいる USB ケーブル経由で通信します。
    *   Poke-Controller からの操作コマンド受信と、デバッグログ送信を同時に行います。
    *   ボーレートは **115200 bps** を推奨しますが、CDC接続のため設定値に関わらず高速に通信できます。

2.  **物理 UART (TX/RX)** (従来互換)
    *   USB-UART アダプタ等を `TX(0)` / `RX(1)` ピンに繋いで操作することも可能です。

---

## USBキーボード機能 (v1.3.3 新機能)

本バージョンより、コントローラー入力だけでなく **USBキーボード入力** に対応しました。
Switch のニックネーム入力画面や、チャット画面などで文字を自動入力できます。

### コマンド仕様

| コマンド  | 引数   | 説明                                 | 例               |
| :-------- | :----- | :----------------------------------- | :--------------- |
| `"`       | 文字列 | 指定した文字列を高速に入力します。   | `"Hello`         |
| `Key`     | Hex    | 指定したキーを 1回押して離します。   | `Key 28` (Enter) |
| `Press`   | Hex    | 指定したキーを押しっぱなしにします。 | `Press 04` (A)   |
| `Release` | (なし) | 押されている全てのキーを離します。   | `Release`        |

> ※ Hex は HID Usage ID (16進数) です。例: `04`=`a`, `05`=`b`, `28`=`Enter`

### [重要] 日本語入力モードの対策

Switch のキーボード画面が「日本語入力（ローマ字入力）」になっていると、英字コマンドを送っても正しく入力されない場合があります。
文字入力を始める前に、**入力モード切替キー** を送ることで回避できます。

```python
# Pythonスクリプト例
# 1. 全角/半角キー (0x35) を送って、強制的に半角英数モードにする
self.keys.ser.ser.write(b'Key 35\n')
self.wait(0.5)

# 2. 文字列を入力
self.keys.ser.ser.write(b'"Hello\n')
```

### [Tips] Keys.py を拡張して便利に使う (上級者向け)

毎回 `ser.write` を書くのが面倒な場合、Poke-Controller 側の `Commands/Keys.py` に以下のメソッドを追加すると便利です。
メソッド名を `type_string` など独自の名前にすることで、他のコントローラー使用時への影響を防げます。

**追加場所**: `Keys.py` の `KeyPress` クラス内

```python
    # RP2040-Zero v1.3.3 専用: キーボード文字列入力
    def type_string(self, text):
        import time
        # 1. 送信用のシリアルオブジェクトを取得 ('Sender'ラッパー対策)
        serial_obj = self.ser
        if hasattr(self.ser, 'ser'):
            serial_obj = self.ser.ser
            
        # 2. 入力モード対策（全角/半角キー送信）
        serial_obj.write(b'Key 35\n')
        time.sleep(0.5)

        # 3. 文字列送信
        cmd = f'"{text}\n'.encode('utf-8')
        serial_obj.write(cmd)
```

**使い方 (Pythonスクリプト)**:
```python
self.keys.type_string("Hello")
```

---

## ドキュメント

本プロジェクトの詳細な解説や、安定化設計については以下の記事を参照してください。

- **[🕹️ PokeControllerForRP2040Zero 完全解説【決定版】](https://note.com/zephel01/n/ne5a9fec85f12)**
  - アーキテクチャ、TinyUSB の設定、プロトコル解析ロジックなど、仕組み全体の詳細な解説です。

- **[RP2040-Zero で作る Nintendo Switch 自動操作コントローラー【v1.3.0 完全版】](https://note.com/zephel01/n/n2c64da9ea81b)**
  - 安定性・信頼性を向上させた v1.3.0 の実装解説（WDT、安全装置、LEDステータスなど）です。

## ライセンス

[MIT License](LICENSE)

---

## 謝辞・参考プロジェクト

本プロジェクトは、以下の素晴らしいオープンソースプロジェクトや情報を参考に作成されました。

- **[Adafruit TinyUSB Library](https://github.com/adafruit/Adafruit_TinyUSB_Arduino)**: RP2040 での柔軟な USB デバイス実装を可能にするライブラリ
- **[sorasen2020/PokeControllerForPico](https://github.com/sorasen2020/PokeControllerForPico)**: RP2040 (Raspberry Pi Pico) を用いた実装の先駆的なプロジェクト
- **[Earle Philhower's Arduino-Pico Core](https://github.com/earlephilhower/arduino-pico)**: Arduino 環境で RP2040 を扱うための強力なコア
- **[HORI PAD](https://hori.jp/)**: コントローラーとしての動作プロファイルの参考
