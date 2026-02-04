# Poke-Controller Python Extensions for RP2040-Zero v1.3.3

このフォルダには、**RP2040-Zero v1.3.3 ファームウェア** の新機能「USBキーボード入力」を、Poke-Controller から便利に使用するための拡張ファイルが含まれています。

## 含まれているファイル

1.  **`Keys.py`**
    *   Poke-Controller の標準ライブラリ（`Commands/Keys.py`）を元に、RP2040-Zero 専用のメソッド `type_string` を追加したものです。
    *   これを使うことで、マクロ内から簡単に文字入力ができるようになります。

2.  **`KeyboardTest.py`**
    *   上記 `Keys.py` が正しく導入されているか確認するためのサンプルマクロです。
    *   Switchのニックネーム入力画面などで実行すると、`Hello` `World` と入力されます。

## 導入方法

1.  **`Keys.py` の配置**
    *   このフォルダにある `Keys.py` を、ご使用の Poke-Controller の `Commands` フォルダ（または `SerialController/Commands`）に**上書きコピー**してください。
    *   ※ 元の `Keys.py` のバックアップを取ることを推奨します。

2.  **`KeyboardTest.py` の配置**
    *   このファイルを、Poke-Controller のユーザーマクロ用フォルダ（`PythonCommands` など）にコピーしてください。

3.  **Poke-Controller の再起動**
    *   ファイルを配置したら、Poke-Controller を再起動（または Reload）してください。

## 使い方（マクロの書き方）

`Keys.py` を導入すると、以下のように書くだけで文字入力が可能になります。
（自動的に日本語入力モード対策が入っているため、Switch側を気にする必要はありません）

```python
class MyMacro(PythonCommand):
    NAME = 'My Macro'

    def do(self):
        # ... 何かの操作 ...

        # 文字列を入力
        self.keys.type_string("Hello")
        
        self.wait(1.0)
        
        # 続きの入力
        self.keys.type_string("World")
```
