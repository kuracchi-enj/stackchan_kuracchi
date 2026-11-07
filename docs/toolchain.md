# このプロジェクトのArduino開発環境

最初はArduinoの仕組みを使い、C++でプログラムを作る。実際のビルドには、こちらから再現しやすいArduino CLIを使う。Arduino IDEは必須ではない。

## 使う組み合わせ

| 項目 | 固定する値 |
| --- | --- |
| PC | Apple SiliconのMac |
| Arduino CLI | 1.5.1、macOS ARM64 |
| ボード定義 | `m5stack:esp32@3.2.2` |
| ボード | `m5stack:esp32:m5stack_cores3` |
| M5StackChan | 1.1.1 |
| M5Unified | 0.2.21 |
| M5GFX | 0.2.28 |

M5Unified 0.2.21の依存宣言に合わせてM5GFX 0.2.28を選ぶ。M5StackChan 1.1.1はM5UnifiedのI/O拡張API変更への互換処理を持つ。単に最新版を組み合わせず、この組み合わせでビルドを確認する。

この最小プログラムではIR・NFCを使わないため、M5StackChanの全サンプル向け依存を自動で追加せず、使う3ライブラリーを明示的に入れる。後でIR・NFCを使うときは、その段階で必要なライブラリーも固定する。

## 初回だけ行う準備

ターミナルでこのプロジェクトのフォルダーを開き、次を実行する。

```bash
./scripts/setup-arduino.sh
```

公式配布のCLIを取得し、SHA-256を確認してから配置する。ボード定義とSDKにはM5Stack公式資料が案内する公式ミラーを使う。通常版とミラー版で、ボードパッケージのチェックサムが同じであることも確認した。

初回はコンパイラーとSDKのダウンロードで時間とディスク容量を使う。普段のビルドでは取得し直さない。

## ビルドする

```bash
./scripts/build-minimal.sh
```

このコマンドはプログラムを本体用に変換するだけ。本体への書き込みは実行しない。

生成物は`build/minimal_offline/`へ入る。`.tools/`、`private/`、`build/`はGit管理対象外。

## 本体なしの回帰テスト

本体なしで初期化判定の回帰テストを行う場合は、MacのCommand Line Toolsに含まれるC++コンパイラーを使う。

```bash
./scripts/test-minimal.sh
```

機器との通信をスタブに置き換え、実際のスケッチの判定処理を実行する。実機での動作確認は別に行う。

## Mac全体の環境と分ける

`scripts/arduino.sh`が次の保存先を設定する。

- CLI本体：`.tools/arduino-cli`
- ボード定義とツール：`private/arduino/data/`
- ダウンロード：`private/arduino/downloads/`
- ライブラリー：`private/arduino/user/libraries/`
- ビルドキャッシュ：`private/arduino/cache/`

通常の`~/Library/Arduino15`や`~/Documents/Arduino`を使わず、このプロジェクトのフォルダー内に置く。

## 参照資料

- [Arduino CLI公式インストール手順](https://docs.arduino.cc/arduino-cli/installation/)
- [M5Stack公式ボード定義の手順とミラー](https://docs.m5stack.com/en/arduino/arduino_board)
- [StackChan公式Arduino手順](https://docs.m5stack.com/en/arduino/stackchan/program)
- [M5StackChanソース](https://github.com/m5stack/StackChan-BSP)

実機での動作と、ここに書いたバージョンでビルドできることは別々に確認する。
