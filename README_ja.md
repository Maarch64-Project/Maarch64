# Maarch64 (マーチ64)

**Maarch64** は、AArch64 (ARM 64-bit) バイナリを x86_64 ホスト環境上でネイティブ同等の速度で直接実行する、プラットフォーム非依存のユーザースペース動的バイナリ変換エンジン（Binary Translator）です。

Linux の **ELF64** バイナリに加えて、macOS の **Mach-O ARM64** バイナリの自動判別ロードにも対応しています。

QEMU などの全文脈エミュレータとは異なり、Maarch64 は **ハイブリッド Cranelift JIT エンジン** と **ゼロコスト・ライブラリ・パススルー（Thunking）** を組み合わせ、共有ライブラリ呼び出し (`glibc`, `libX11`, `libEGL`, `OpenGL`, `ALSA`, `PulseAudio`, `LibVLC`, `Metal`) をホスト x86_64 のネイティブライブラリへ直接ブリッジ処理することで、極めて高い描画・実行パフォーマンスを実現します。

---

## 🏗️ ワークスペース構成

Maarch64 は `Maarch64-Project` GitHub 組織配下の Git サブモジュールによって設計されたモジュール式 Rust Cargo ワークスペースです：

| サブモジュール | リポジトリ | 説明 |
| :--- | :--- | :--- |
| **`core`** | [`maarch64-core`](https://github.com/Maarch64-Project/maarch64-core) | コアエンジン: `CpuContext`, `MemoryManager`, `AutoLoader` (ELF64 & Mach-O ARM64), `SyscallDispatcher` (Linux & Darwin BSD/Mach Traps), Cranelift JIT コンパイラ, `bad64` インタプリタ。 |
| **`thunks`** | [`maarch64-thunks`](https://github.com/Maarch64-Project/maarch64-thunks) | パススルーライブラリブリッジ (`gpu`, `audio`, `vlc`, `darwin` ObjC, `metal`)。 |
| **`tools`** | [`maarch64-tools`](https://github.com/Maarch64-Project/maarch64-tools) | CLI ツール: `maarch64-runner` (バイナリ実行器), `maarch64-diff-test` (Unicorn Engine 差分検証器)。 |
| **`tests`** | [`maarch64-tests`](https://github.com/Maarch64-Project/maarch64-tests) | AArch64 テスト用フィクスチャ (`asm/`, `c/`, `rust/`), クロスコンパイルスクリプト, 統合テスト。 |
| **`docs`** | [`maarch64-docs`](https://github.com/Maarch64-Project/maarch64-docs) | アーキテクチャ仕様書および技術ドキュメント。 |

---

## 📚 技術ドキュメント

- 📐 **[全体アーキテクチャ仕様書 (ARCHITECTURE.md)](docs/ARCHITECTURE.md)**: 全体システム構造とコンポーネント構成図。
- 🌐 **[マルチOSバイナリ変換仕様 (MULTI_OS.md)](docs/MULTI_OS.md)**: ELF64 vs Mach-O ARM64 ロード, System V vs Darwin ABI, Linux & Darwin システムコールルーティング。
- ⚡ **[Cranelift JIT エンジン仕様 (JIT_ENGINE.md)](docs/JIT_ENGINE.md)**: 動的コンパイルパイプライン, レジスタマッピング, 対応命令セット, インタプリタフォールバック。
- 🔌 **[ライブラリ・パススルー仕様 (THUNKS.md)](docs/THUNKS.md)**: ホスト GPU (EGL/GLX), オーディオ (ALSA/Pulse), LibVLC, Metal フレームワーク パススルー構造。

---

## 🚀 クイックスタート & 実行方法

### 1. リポジトリのクローン

```bash
git clone --recursive git@github.com:Maarch64-Project/Maarch64.git
cd Maarch64
```

### 2. ワークスペースのビルド

```bash
cargo build --workspace
```

### 3. ARM64 バイナリの実行

```bash
# インタプリタモードで実行
./target/debug/maarch64 ./sysroot/aarch64-rootfs/usr/bin/vlc

# Cranelift JIT アクセラレーション有効化で実行
./target/debug/maarch64 --jit ./sysroot/aarch64-rootfs/usr/bin/vlc
```

---

## 📄 ライセンス

Dual-licensed under either of:
- Apache License, Version 2.0 ([LICENSE-APACHE](LICENSE-APACHE))
- MIT License ([LICENSE-MIT](LICENSE-MIT))
