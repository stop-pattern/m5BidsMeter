# m5BidsMeter

M5Stack Core2 向けの PlatformIO ファームウェア開発リポジトリです。PC の鉄道運転ゲームから BIDS v202 形式の情報を USB シリアル通信で受信し、E233 系 TIMS 風の画面に速度、BC/MR 圧、ブレーキ段数、保安装置の状態を表示します。現在の `src/main.cpp` は Arduino の生成時サンプルで、製品機能はまだ実装されていません。開発環境を確認するための実機テストは `test/` にあります。

## リポジトリの構成

| パス | 役割 |
| --- | --- |
| `platformio.ini` | `m5stack-core2`、`espressif32`、Arduino のビルド設定 |
| `src/`、`include/`、`lib/` | アプリケーションコード、共有ヘッダー、プロジェクト固有ライブラリ |
| `test/` | PlatformIO の実機環境テスト |
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

確定した表示要件と実装前に決める項目は要件メモを参照してください。作業中の仕様・判断・進捗は対応する文書と作業ログに残し、変更は検証できる小さな単位でコミットします。
