# m5BidsMeter

M5Stack Core2 向けの PlatformIO ファームウェア開発リポジトリです。`src/main.cpp` は現在 Arduino の生成時サンプルで、メーター機能とテストはまだありません。製品の用途や表示内容は未確定です。

## リポジトリの構成

| パス | 役割 |
| --- | --- |
| `platformio.ini` | `m5stack-core2`、`espressif32`、Arduino のビルド設定 |
| `src/`、`include/`、`lib/` | アプリケーションコード、共有ヘッダー、プロジェクト固有ライブラリ |
| `test/` | PlatformIO テストの配置先。現在は説明用 README のみ |
| [AGENTS.md](AGENTS.md) | Codex の作業・記録・コミット指針 |
| [要件メモ](docs/requirements.md) | 確定した製品仕様と未決定事項 |
| [開発ガイド](docs/development.md) | 環境、ビルド、実機書き込み、検証方法 |
| [作業ログ](docs/work-log.md) | 作業経過と再開時の引き継ぎ |

## ビルド

PlatformIO Core が使える環境で、リポジトリのルートから実行します。

```powershell
pio run -e m5stack-core2
```

必要な環境と実機での確認方法は開発ガイドを参照してください。

## 開発の依頼

機能開発では、入力元、画面に出す値、更新周期、エラー時の動作、完了条件を指定してください。作業中の仕様・判断・進捗は対応する文書と作業ログに残し、変更は検証できる小さな単位でコミットします。
