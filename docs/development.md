# 開発ガイド

## 対象と準備

対象は M5Stack Core2、PlatformIO 環境名は `m5stack-core2` です。設定はリポジトリ直下の `platformio.ini` にあります。以下のコマンドは PowerShell でリポジトリのルートから実行します。

PlatformIO Core の `pio` が PATH にある場合は `pio` を使用できます。PATH にない Windows 環境では、標準的なユーザー領域のインストール先を次のように指定します。

```powershell
$pio = Join-Path $env:USERPROFILE '.platformio\penv\Scripts\pio.exe'
& $pio --version
```

この場所にも存在しない場合は PlatformIO Core または PlatformIO IDE のインストールを確認してください。初回ビルドやテストではプラットフォーム、ツールチェーン、Unity の取得が必要になる場合があります。

## Core2 のポートを特定する

Core2 を USB で接続し、ポート一覧を確認します。

```powershell
& $pio device list
$port = 'COM<number>'  # 一覧から特定した Core2 のポートに置き換える
```

Core2 の USB シリアル変換チップには CP2104 または CH9102F の機種があります。[M5Stack の Core2 資料](https://docs.m5stack.com/en/core/core2)を参照し、表示されたデバイス名と接続・切断時の変化で対象ポートを確認してください。ポート番号は PC ごとに変わるため、`platformio.ini` に固定しません。デバイスのシリアル番号や MAC アドレスをログ・コミットに含めないでください。

## 通常のファームウェアをビルド・書き込み

```powershell
& $pio run -e m5stack-core2
& $pio run -e m5stack-core2 -t upload --upload-port $port
```

ビルド成功と書き込み成功は別々に確認します。書き込みコマンドは接続中の Core2 のファームウェアを置き換えます。製品ファームウェアは 115200 bps で BIDS 情報を照会します。

## PC 上の状態判定テスト

PC 用 C++ コンパイラがない Windows 環境では、PlatformIO の `toolchain-gccmingw32` をユーザー領域へ導入する。子プロセスが DLL を見つけられるよう、`bin` をそのコマンドの PATH に追加する。

```powershell
& $pio pkg install --global --tool 'platformio/toolchain-gccmingw32'
$compilerBin = Join-Path $env:USERPROFILE '.platformio\packages\toolchain-gccmingw32\bin'
$env:PATH = "$compilerBin;$env:PATH"
& (Join-Path $compilerBin 'g++.exe') -std=c++11 -Wall -Wextra -Iinclude src/meter.cpp test/test_model.cpp -o .pio/test_model.exe
& .pio/test_model.exe
```

両コマンドの終了コードが 0 なら、BIDS 応答の解析と状態判定が成功している。

## 実機テストと双方向シリアル通信

`test/test_environment/test_main.cpp` は ESP32 上で Unity テストを実行し、その後は `PING` に `PONG` と返す診断用ファームウェアです。次の手順では一時的にこのテストファームウェアを書き込みます。

```powershell
& $pio test -e m5stack-core2 --upload-port $port --test-port $port -f test_environment
& $pio device monitor -p $port -b 115200
```

テストでは `test_esp32_runtime_has_free_heap` の `[PASSED]` と、サマリーの成功件数を確認します。モニタが開いたら `PING` を入力して Enter を押し、`PONG` が返ることを確認します。`Ctrl+C` でモニタを終了します。通信の確認後は、上記の通常ファームウェアの書き込みコマンドを再実行して戻します。モニタを閉じてから書き込んでください。同じポートを同時に開くことはできません。

このテストは PlatformIO のビルド、書き込み、実機上の Unity 実行、USB シリアルの双方向通信を確認します。画面、タッチ操作、BIDS 通信、PC ゲームとの連携は確認しません。製品の検証では模擬 BIDS 送信機を使用し、テスト後は製品ファームウェアを書き戻してください。

## 参照資料

- [PlatformIO の `pio run`](https://docs.platformio.org/en/latest/core/userguide/cmd_run.html)
- [PlatformIO の `pio test`](https://docs.platformio.org/en/latest/core/userguide/cmd_test.html)
- [PlatformIO のシリアルモニタ](https://docs.platformio.org/en/stable/core/userguide/device/cmd_monitor.html)
