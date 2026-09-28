# 開発ガイド

## 環境

- 対象ボード: M5Stack Core2
- ビルド環境名: `m5stack-core2`
- PlatformIO 設定: ルートの `platformio.ini`
- フレームワーク: Arduino（ESP32）

PlatformIO Core の `pio` コマンド、または PlatformIO IDE を使用します。バージョンや追加ライブラリは現時点で固定されていません。初回ビルドでは PlatformIO が必要なプラットフォームとツールチェーンを取得します。

## ビルド

リポジトリのルートで実行します。

```powershell
pio run -e m5stack-core2
```

成功時は対象環境のビルドが完了します。ビルドだけでは実機動作は確認できません。`pio` が見つからない場合は PlatformIO Core の導入または IDE 側のターミナル環境を確認してください。

## 実機への書き込み

M5Stack Core2 を接続し、対象ポートを確認してから実行します。

```powershell
pio device list
pio run -e m5stack-core2 -t upload
```

複数のシリアル機器がある場合は、対象を確認したうえで `--upload-port` を指定してください。自動化した作業では、利用者が実機書き込みを求めている場合にのみ実行します。

## テストと変更時の確認

現在、`test/` には PlatformIO の説明用 README だけがあり、実行できるテストはありません。機能を追加する際は、ハードウェアなしで確認できるロジックには適切なテストを追加してください。実機に依存する機能は、必要な機材、操作、期待結果を変更報告に記録してください。ビルド、テスト、実機確認の結果は区別して報告します。

## 参照資料

- [PlatformIO の `pio run`](https://docs.platformio.org/en/latest/core/userguide/cmd_run.html)
- [PlatformIO の `pio test`](https://docs.platformio.org/en/latest/core/userguide/cmd_test.html)
