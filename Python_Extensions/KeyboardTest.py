from Commands.PythonCommandBase import PythonCommand
from Commands.Keys import KeyPress, Button, Hat, Direction, Stick
import time

class KeyboardTest(PythonCommand):
    NAME = 'キーボード入力テスト (v1.3.3)'

    def __init__(self, *args):
        super().__init__(*args)

    def do(self):
        print("--------------------------------------------------")
        print("RP2040-Zero v1.3.3 キーボード入力デモ")
        print("※ 事前に拡張版 Keys.py を導入している必要があります。")
        print("--------------------------------------------------")
        print("3秒後に開始します...")
        self.wait(3.0)

        # 拡張メソッド type_string を使用
        # 内部で「入力モード切替(全角/半角)」を行ってから文字列を入力するため、
        # Switchが日本語入力モードのままでも正しく英字が入力されます。

        if hasattr(self.keys, 'type_string'):
            print("Sending: Hello")
            self.keys.type_string("Hello")
            self.wait(1.0)
            
            print("Sending: Enter Key (using direct serial for Special Key)")
            # 特殊キーはまだメソッド化していないので直接叩くか、別途実装可能
            # ここでは標準の入力テストとして文字列のみデモ
            
            # Enterキー (0x28)
            if hasattr(self.keys.ser, 'ser'):
                 self.keys.ser.ser.write(b'Key 28\n')
            else:
                 self.keys.ser.write(b'Key 28\n')
                 
            self.wait(1.0)

            print("Sending: World")
            self.keys.type_string("World")
        
        else:
            print("エラー: self.keys.type_string が見つかりません。")
            print("同梱の Keys.py を Poke-Controller の Commands フォルダに上書きしてください。")

        print("--------------------------------------------------")
        print("テスト終了")
