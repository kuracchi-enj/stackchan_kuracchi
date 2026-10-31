# StackChanカスタマイズの実現性調査

調査日：2026-10-31。リポジトリの情報はGitHub連携、製品・プロトコル・AWSの仕様は公式Web資料で確認した。

この調査は文書と公開ソースの確認。購入した実物の型番、実機動作、AWSアカウント、既存MCPサーバーは確認していない。

## 判断の要点

| 項目 | 確認した内容 | 計画への反映 |
| --- | --- | --- |
| 最初の開発環境 | 公式にStackChan用Arduino手順とサンプルがある | Arduino IDE＋C++を第一候補にする |
| 想定機種 | 公式完成品はCoreS3系のESP32-S3を使用 | 実物の製品名・基板・サーボ構成の一致を最初に確認する |
| 復旧 | カスタム後にM5Burnerで公式ファームウェアを書き戻す案内がある | 初回書き込み前に復旧用ファームウェアと手順を確保する |
| PicoRuby | 完成品を対象とした個人移植がある | ゼロから移植せず、対象と依存関係を確認して利用する |
| PicoRubyでの通信 | 移植版の機能表でWi-Fi・HTTP・MQTT・WebSocketは未接続 | 第3段階に通信の検証・追加工数を置く |
| MQTTとMCP | 標準のMCP通信方式にMQTTは含まれない | AWS側にMCPクライアントを持つ橋渡し処理を置く |
| MCPの版 | 2026-07-28版と2025-11-25以前で接続開始の方式が異なる | 既存サーバーとSDKの対応版を確認し、版に合った処理を使う |

## PicoRubyについて確認した範囲

参照した[stackchan-picorubyのコミット](https://github.com/bash0C7/stackchan-picoruby/commit/9fac4446ba73f46d09fbbf89e55ed22f11e323b3)は`9fac4446ba73f46d09fbbf89e55ed22f11e323b3`。

その[README](https://github.com/bash0C7/stackchan-picoruby/blob/9fac4446ba73f46d09fbbf89e55ed22f11e323b3/README.md)には次の記載がある。

- 対象はM5Stack StackChan AI Desktop Robot。Switch Scienceの商品11129へのリンクがある。
- 個人による移植で、開発途中。APIやビルド方法は変わり得る。
- 表情、LED、首のサーボ、頭のタッチ、BLE、スピーカーは対応として掲載されている。
- マイク、カメラ、IMU、NFC、Wi-Fi・HTTP・MQTT・WebSocketは未対応として掲載されている。
- 開発環境はmacOS、Ruby 4.0以上、ESP-IDF v5.4、Swiftを含むXcodeなどを前提にしている。
- Mac側との通信はBLE上のdRuby。任意の外部端末から自由に呼ばせる設計にはしない。
- スタックの余裕が小さく、例外処理でオーバーフローし得る既知の問題がある。

「対応」は移植作者の記載。こちらの実機で確認できたという意味ではない。また、移植版の未対応はStackChan本体のハードウェア制限を意味しない。

[ビルド設定](https://github.com/bash0C7/stackchan-picoruby/blob/9fac4446ba73f46d09fbbf89e55ed22f11e323b3/build_config/esp32-stackchan.rb)にはnetwork・socket・mbedtlsのgem参照や`USE_WIFI`の定義がある。ただし、gem名が存在するだけでは、StackChan上のWi-Fi＋TLS＋MQTTの組み合わせが動く証拠にならない。

[Rakefile](https://github.com/bash0C7/stackchan-picoruby/blob/9fac4446ba73f46d09fbbf89e55ed22f11e323b3/Rakefile)では、`r2p2:upload_appmrb`が`wipe_storage`を呼び出す。Rubyアプリだけを更新するつもりでも、ストレージが消える経路がある。秘密情報を保存する領域と更新手順の確認が必要。

[依存関係のロック](https://github.com/bash0C7/stackchan-picoruby/blob/9fac4446ba73f46d09fbbf89e55ed22f11e323b3/acceptance/lock.yml)には、作者のR2P2-ESP32・PicoRubyのforkや複数のドライバーのSHAが記録されている。まずこの組み合わせを再現し、upstream最新版と一度に混ぜない。

一方、[upstream R2P2-ESP32のREADME](https://github.com/picoruby/R2P2-ESP32/blob/64bb79002617af2f70be57dab19fa6dbfaffdcc1/README.md)は、CoreS3の対応、ESP-IDF v5.5でのビルド、`USE_WIFI`によるWi-Fi有効化を案内している。これは別の組み合わせの情報であり、StackChan移植へのそのままの適用は未検証。

## MQTTとMCPの接続について

StackChanはMQTTで小さなJSONを送る。AWS IoT Coreが受け取り、IoT RuleからLambdaを起動する。Lambda内のMCPクライアントが自前MCPサーバーのツールを呼び、結果をMQTTで返す構成にする。

MQTTメッセージをMCPサーバーへそのまま送るだけでは、MCPのバージョン・通信方式・認証・応答処理を満たせない。MCP SDKを使う。

MCPの[2026-07-28版のバージョン仕様](https://modelcontextprotocol.io/specification/2026-07-28/basic/versioning)では、毎回のリクエストに版などのメタデータを含め、初期化ハンドシェイクを使わない。[2025-11-25版のライフサイクル](https://modelcontextprotocol.io/specification/2025-11-25/basic/lifecycle)では`initialize`などが必要。新旧どちらかを、相手とSDKの対応に合わせて固定する。

IoT RuleからのLambda呼び出しは非同期。MQTTの再送だけでなくLambda側でも同じイベントが複数回来る可能性があるため、`request_id`で処理を管理する。Lambdaの戻り値が自動的にMQTT返信になるわけではない。IoT Data PlaneのPublishを明示的に実行する。

## 参照資料

| ID | 一次資料 | 使用箇所 |
| --- | --- | --- |
| S1 | [M5Stack StackChan製品資料](https://docs.m5stack.com/en/StackChan) | 製品構成、取り扱い、2.4GHz Wi-Fi、公式ファームウェアの復旧 |
| S2 | [StackChan Arduino書き込み手順](https://docs.m5stack.com/en/arduino/stackchan/program) | Arduino IDE、M5CoreS3、M5StackChanライブラリー |
| S3 | [StackChan Display](https://docs.m5stack.com/en/arduino/stackchan/display) | 画面用ライブラリーとサンプルの最低バージョン |
| S4 | [StackChan Servo](https://docs.m5stack.com/en/arduino/stackchan/servo) | 原点とサーボの扱い、上下5〜85度の推奨範囲 |
| S5 | [StackChan RGB LED](https://docs.m5stack.com/en/arduino/stackchan/rgb) | 最初のLED動作確認 |
| S6 | [stackchan-picoruby README・固定版](https://github.com/bash0C7/stackchan-picoruby/blob/9fac4446ba73f46d09fbbf89e55ed22f11e323b3/README.md) | 対象機種、対応機能、開発環境、既知の問題 |
| S7 | [同ビルド設定・固定版](https://github.com/bash0C7/stackchan-picoruby/blob/9fac4446ba73f46d09fbbf89e55ed22f11e323b3/build_config/esp32-stackchan.rb) | 組み込むgem、通信部分の調査候補 |
| S8 | [同Rakefile・固定版](https://github.com/bash0C7/stackchan-picoruby/blob/9fac4446ba73f46d09fbbf89e55ed22f11e323b3/Rakefile) | ビルド・書き込み・ストレージ消去の挙動 |
| S9 | [同依存ロック・固定版](https://github.com/bash0C7/stackchan-picoruby/blob/9fac4446ba73f46d09fbbf89e55ed22f11e323b3/acceptance/lock.yml) | 再現する依存関係 |
| S10 | [upstream R2P2-ESP32 README・固定版](https://github.com/picoruby/R2P2-ESP32/blob/64bb79002617af2f70be57dab19fa6dbfaffdcc1/README.md) | CoreS3対応、Wi-Fiの有効化。移植版とは区別 |
| S11 | [AWS IoT通信方式](https://docs.aws.amazon.com/iot/latest/developerguide/protocols.html) | MQTT/TLS、X.509、8883番ポート |
| S12 | [AWS IoT MQTT](https://docs.aws.amazon.com/iot/latest/developerguide/mqtt.html) | QoS、セッション、保持メッセージ |
| S13 | [AWS IoTポリシー例](https://docs.aws.amazon.com/iot/latest/developerguide/pub-sub-policy.html) | Connect・Publish・Subscribe・Receiveの分離 |
| S14 | [ESP-IDF v5.4 ESP-MQTT API](https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32/api-reference/protocols/mqtt.html) | Cで通信を追加する際の候補。ESP32向け資料で、S3用ビルドは別途検証 |
| S15 | [AWS IoTのLambdaアクション](https://docs.aws.amazon.com/iot/latest/developerguide/lambda-rule-action.html) | 非同期呼び出しとLambdaへのリソースポリシー |
| S16 | [Lambda非同期呼び出しのエラー処理](https://docs.aws.amazon.com/lambda/latest/dg/invocation-async-error-handling.html) | 再試行、重複、失敗時の保存 |
| S17 | [MCP 2026-07-28通信方式](https://modelcontextprotocol.io/specification/2026-07-28/basic/transports) | stdioとStreamable HTTP、独自通信方式との区別 |
| S18 | [MCP 2026-07-28のバージョン仕様](https://modelcontextprotocol.io/specification/2026-07-28/basic/versioning) | 新しい版のリクエスト処理 |
| S19 | [MCP 2025-11-25ライフサイクル](https://modelcontextprotocol.io/specification/2025-11-25/basic/lifecycle) | 以前の版の初期化と接続処理 |
| S20 | [AgentCore RuntimeへのMCPサーバー配置](https://docs.aws.amazon.com/bedrock-agentcore/latest/devguide/runtime-mcp.html) | MCPサーバーを新規に用意する場合の標準案 |
| S21 | [AgentCore MCPプロトコル契約](https://docs.aws.amazon.com/bedrock-agentcore/latest/devguide/runtime-mcp-protocol-contract.html) | ARM64、8000番、/mcp、セッションヘッダー |
| S22 | [AgentCore Runtimeのセキュリティ](https://docs.aws.amazon.com/bedrock-agentcore/latest/devguide/runtime-security-best-practices.html) | AWS内の呼び出しでのIAM認証、権限の限定 |

資料は実装開始時にも差分を確認する。最新への更新は、動いている組み合わせを記録したうえで一つずつ行う。
