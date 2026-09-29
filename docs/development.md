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

Core2 を USB で接続し、左側の電源ボタンを 1 回押して起動します。必要なら底面の RST ボタンを 1 回押してリセットします。画面が消灯している場合は既存ファームウェアの動作だけでは電源状態を判断できないため、本体の表示・電源ランプとポートの接続状態を確認します。ポート一覧を確認します。

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
& (Join-Path $compilerBin 'g++.exe') -std=c++11 -Wall -Wextra -Iinclude src/frame_diff.cpp src/meter.cpp src/render.cpp src/render_layout.cpp src/render_gauges.cpp src/render_safety.cpp test/test_model.cpp -o .pio/test_model.exe
& .pio/test_model.exe
```

両コマンドの終了コードが 0 なら、BIDS 応答の解析、状態判定、画面遷移のテストが成功しています。

## PC 上の画面プレビュー

状態判定テストと同じ C++ 描画処理を Windows GDI で表示用画像に変換します。前節の `$compilerBin` と PATH の設定を使います。

```powershell
& (Join-Path $compilerBin 'g++.exe') -std=c++11 -Wall -Wextra -Iinclude src/meter.cpp src/render.cpp src/render_layout.cpp src/render_gauges.cpp src/render_safety.cpp tools/preview.cpp -lgdi32 -o .pio/preview.exe
& .pio/preview.exe
```

`preview/` に 13 枚の BMP ができます。通常・通信断・音量 50%・消音のホーム、速度、BC 警告の赤・通常状態、ブレーキ、3 種類の保安装置、選択画面を目視確認します。`preview/` はコミットしません。

## 描画とコードの構成

状態判定、シリアル照会、入力、画面描画、Core2 の液晶転送はそれぞれ独立した関数群に分けます。共通描画処理は PC プレビューと実機で共有します。実機では画面全体をオフスクリーン画像に描き、前回画像と行帯単位で比較します。異なる行帯だけを液晶へ転送し、変化がない回は転送しません。画面切替時は新しい画面を一度に組み立ててから転送します。描画間隔より短い BIDS 照会周期は維持します。

日本語は読みやすいゴシック系フォントを選びます。表示灯の縦書きは長音記号を縦線にし、長い文字列は右列から左列へ折り返して枠内に収めます。各モジュールの公開 API、関数、状態を表す変数には Doxygen コメントを付け、責務を明示します。

画面共通の振り分けは `src/render.cpp`、ホームと画面切替は `src/render_layout.cpp`、計器は `src/render_gauges.cpp`、灯群は `src/render_safety.cpp` に置きます。ホームの項目追加時は `include/meter.h` の画面種別、`src/render.cpp` の振り分け、`src/render_layout.cpp` の行先・表示名一覧を合わせて更新します。ホームは 1 ページ 4 行で、項目数からページ数を求めます。実機固有の USB 通信、操作入力、差分転送は `src/bids_transport.cpp`、`src/core2_input.cpp`、`src/core2_display.cpp` に分けます。操作音は `src/core2_input.cpp` の `kClickHz` と `kClickMs`、`include/core2_input.h` の `kSoundFullVolume` を変えて周波数、長さ、100% の音量を調整します。起動時と操作時の音量は同じ最大音量定数を使用します。コード整形設定は `.clang-format` にあります。

## 模擬 BIDS 送信機

製品ファームウェアを書き込んだ Core2 と、ポートを開ける PC に Python と pyserial を用意します。pyserial がなければ `python -m pip install pyserial` で導入します。

```powershell
python tools/mock_bids.py --port $port --version 202 --duration 16 --reset
python tools/mock_bids.py --port $port --version 100 --duration 16 --reset
```

送信機は Core2 の `TRIE1/E3/E4/H0/H1` と `TRIPn` 照会に応答します。`--reset` はポートを開いた後に Core2 を再起動し、起動時の `TRV202` 版照会から確認します。5 秒ごとに走行、速度 0・BC 150 kPa の転動防止、回復を循環します。終了時に版照会、五つの基本照会の受信回数と Panel 照会の総数を表示し、基本照会が一つでも欠ければ失敗終了します。保安装置画面と CS-ATC の速度画面は本体で選択して Panel 照会を確認します。実画面の表示・点滅・操作も本体を目視して確認します。送信機とシリアルモニタは同時に開けません。

## 実機テストと双方向シリアル通信

`test/test_environment/test_main.cpp` は ESP32 上で Unity テストを実行し、その後は `PING` に `PONG` と返す診断用ファームウェアです。次の手順では一時的にこのテストファームウェアを書き込みます。

```powershell
& $pio test -e m5stack-core2 --upload-port $port --test-port $port -f test_environment
& $pio device monitor -p $port -b 115200
```

テストでは `test_esp32_runtime_has_free_heap` の `[PASSED]` と、サマリーの成功件数を確認します。モニタが開いたら `PING` を入力して Enter を押し、`PONG` が返ることを確認します。`Ctrl+C` でモニタを終了します。通信の確認後は、上記の通常ファームウェアの書き込みコマンドを再実行して戻します。モニタを閉じてから書き込んでください。同じポートを同時に開くことはできません。

このテストは PlatformIO のビルド、書き込み、実機上の Unity 実行、USB シリアルの双方向通信を確認します。画面、タッチ操作、BIDS 通信、PC ゲームとの連携は確認しません。製品の検証では模擬 BIDS 送信機を使用し、テスト後は製品ファームウェアを書き戻してください。

書き込み時に `Failed to connect to ESP32: No serial data received` と出る場合は、対象ポートの接続・切断による変化、本体電源、ほかのアプリによるポート占有を確認します。ポート一覧の名称だけでは対象機を確定できません。必要に応じて本体のリセットを試し、同じポートで再実行します。書き込み成功前に実機テスト成功とは記録しません。

## 参照資料

- [PlatformIO の `pio run`](https://docs.platformio.org/en/latest/core/userguide/cmd_run.html)
- [PlatformIO の `pio test`](https://docs.platformio.org/en/latest/core/userguide/cmd_test.html)
- [PlatformIO のシリアルモニタ](https://docs.platformio.org/en/stable/core/userguide/device/cmd_monitor.html)
