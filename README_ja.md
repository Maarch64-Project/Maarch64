# Maarch64 (日本語ドキュメント)

**Maarch64** は、x86_64 ホスト環境上で AArch64 (ARM64) Linux バイナリを高速に実行するための**ユーザー空間バイナリトランスレータ / エミュレータ**です。

QEMU などのフルシステムエミュレータとは異なり、システムライブラリ呼び出し (`glibc`, `libX11`, `OpenGL` 等) をホスト (x86_64) のネイティブライブラリへダイレクトに転送する **Library Wrapping (Thunking)** 手法を採用し、ネイティブに近い超高速な動作を目指しています。

---

## 🏗️ リポジトリ構成

Maarch64 は、GitHub 組織 `Maarch64-Project` の配下で Git サブモジュールを活用したモジュール構造の Rust ワークスペースとして管理されています。

| サブモジュール | リポジトリ | 概要・役割 |
| :--- | :--- | :--- |
| **`core`** | [`maarch64-core`](https://github.com/Maarch64-Project/maarch64-core) | コアエンジン: CPU状態 (`CpuContext`), `mmap` 仮想メモリ (`MemoryManager`), ELF64ローダー (`ElfLoader`), AArch64デコーダ/インタプリタ, Linuxシステムコール変換 (`SyscallDispatcher`) |
| **`thunks`** | [`maarch64-thunks`](https://github.com/Maarch64-Project/maarch64-thunks) | ライブラトラップ・ラッパー (`glibc` 関数パススルー基盤) |
| **`tools`** | [`maarch64-tools`](https://github.com/Maarch64-Project/maarch64-tools) | CLIツール群: バイナリ実行用 `maarch64-runner`, Unicorn Engine 差分検証用 `maarch64-diff-test` |
| **`tests`** | [`maarch64-tests`](https://github.com/Maarch64-Project/maarch64-tests) | AArch64 テストソース (`asm/`, `c/`, `rust/`), クロスコンパイル生成スクリプト (`build_fixtures.sh`), 統合ユニットテスト |
| **`docs`** | [`maarch64-docs`](https://github.com/Maarch64-Project/maarch64-docs) | 設計仕様書・アーキテクチャドキュメント |

---

## 🛠️ 事前準備 (開発環境のセットアップ)

ビルドに必要な依存パッケージおよび AArch64 クロスコンパイラをインストールします (Ubuntu/Debian):

```bash
# 1. 必要パッケージと AArch64 クロスコンパイラのインストール
sudo apt-get update
sudo apt-get install -y gcc-aarch64-linux-gnu libunicorn-dev

# 2. Rust ツールチェーンに AArch64 ターゲットを追加
rustup target add aarch64-unknown-linux-gnu
```

---

## 🚀 基本的な開発・テスト手順 (ワークフロー)

### 1. サブモジュールを含めてリポジトリをクローン

```bash
git clone --recursive git@github.com:Maarch64-Project/Maarch64.git
cd Maarch64
```

すでにクローン済みの場合は、以下で最新状態に更新します：
```bash
git submodule update --init --recursive
```

### 2. テスト用 AArch64 バイナリを一括ビルド

`tests/fixtures/` 配下のテストソースコード (`asm/*.s`, `c/*.c`, `rust/*.rs`) を `tests/bin/` へクロスコンパイルします：

```bash
bash tests/build_fixtures.sh
```

### 3. 単体テスト・統合テストの実行

ワークスペース全体の Rust テストを実行します：

```bash
cargo test --workspace
```

### 4. Unicorn Engine との命令ステップ差分検証テスト

Unicorn Engine (ハードウェアレベルの AArch64 リファレンス) と `maarch64-core` のレジスタ状態を1ステップずつ比較検証します：

```bash
cargo run -p maarch64-diff-test
```

### 5. ARM64 バイナリを Runner で実行

クロスコンパイルした AArch64 バイナリを指定して、Maarch64 エミュレータ上で実行します：

```bash
# アセンブリのテストバイナリを実行
cargo run -p maarch64-runner -- tests/bin/hello_asm

# Rust のテストバイナリを実行
cargo run -p maarch64-runner -- tests/bin/hello_rust
```

---

## 🌿 ブランチ・開発運用ルール

- **`main`**: 安定版リリースブランチ
- **`dev`**: 開発・統合ブランチ
- **開発フロー**:
  1. `dev` から機能ブランチを作成: `git checkout dev && git checkout -b feature/issue-#-name`
  2. 機能実装および `tests/` 内にテストコードを追加
  3. `dev` ブランチ宛てに Pull Request を作成し、自動 CI 検証後にマージ
