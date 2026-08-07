# 🚀 Maarch64: マルチOS AArch64 動的バイナリ変換エンジン

<div align="center">

[![Rust](https://img.shields.io/badge/Rust-2021%20Edition-orange.svg)](https://www.rust-lang.org/)
[![Architecture](https://img.shields.io/badge/Architecture-AArch64%20%E2%86%92%20x86__64-blue.svg)]()
[![Target OS](https://img.shields.io/badge/Target%20OS-Linux%20%7C%20macOS%20Darwin-green.svg)]()
[![JIT Engine](https://img.shields.io/badge/JIT%20Engine-Cranelift-purple.svg)](https://cranelift.dev/)
[![License](https://img.shields.io/badge/License-MIT%20%2F%20Apache--2.0-red.svg)]()

**Maarch64** は、AArch64 (ARM 64-bit) バイナリを x86_64 ホスト環境上でネイティブ同等の速度で直接実行する、プラットフォーム非依存のユーザースペース動的バイナリ変換エンジン（Binary Translator）です。

Linux の **ELF64** バイナリに加えて、macOS の **Mach-O ARM64** バイナリの自動判別ロードにも対応しています。

</div>

---

## 🌟 主な革新性と設計思想

QEMU などの従来型フルシステムエミュレータは、エミュレートされた CPU コンテキスト内で全ライブラリ呼び出しを愚直に解釈・変換するため、グラフィック描画の重いアプリやオーディオ処理で致命的な速度低下を引き起こします。

Maarch64 は、**ハイブリッド Cranelift JIT エンジン** と **ゼロコスト・ライブラリ・パススルー（Thunking）** を融合させることで、この問題を解決しました：

```
+---------------------------------------------------------------------------------------------------+
|                                     Maarch64 の実行パラダイム                                      |
|                                                                                                   |
|   Target ARM64 アプリ計算ロジック ---> [ Cranelift JIT エンジン ] ---> ホストCPU上で直接高速実行  |
|   共有ライブラリ呼び出し (GL/VLC/Audio) -> [ パススルー Thunk ブリッジ ] -> ホスト x86_64 ドライバ   |
+---------------------------------------------------------------------------------------------------+
```

- **⚡ ネイティブ並みの爆速実行**: システムライブラリ (`glibc`, `libX11`, `OpenGL`, `ALSA`, `PulseAudio`, `LibVLC`, `Metal`) の呼び出しをエミュレーションなしでホスト x86_64 ハードウェアへダイレクト転送。
- **🌐 マルチOSバイナリ自動識別**: `AutoLoader` により、Linux ELF64 と macOS Mach-O 64-bit のバイナリ形式をマジックバイトから自動判定してロード。
- **🍏 逆 Rosetta 2 構想**: Intel Mac (macOS x86_64 ホスト) 上で Apple Silicon (ARM64) アプリを 1:1 ネイティブ `.dylib` フォワーディングで動かす動作モデルを想定。
- **🔬 ビット精度検証**: Unicorn Engine オラクルによる命令ステップごとのレジスタ差分検証 (`maarch64-diff-test`) を搭載。

---

## 🏗️ 全体システムアーキテクチャ

```
                                +-----------------------------------+
                                |    Target ARM64 Binary File       |
                                |  (Linux ELF64 / macOS Mach-O 64)  |
                                +-----------------------------------+
                                                  |
                                                  v
                                +-----------------------------------+
                                |            AutoLoader             |
                                |    (Magic-byte Format Detection)  |
                                +-----------------------------------+
                                         /                 \
                         (Linux ELF64)  /                   \  (macOS Mach-O)
                                       v                     v
                        +--------------------+         +--------------------+
                        |     ElfLoader      |         |    MachOLoader     |
                        | (Segment/Stack Map)|         | (Mach-O Stack/ABI) |
                        +--------------------+         +--------------------+
                                       \                     /
                                        v                   v
+---------------------------------------------------------------------------------------------------+
|                                          maarch64-core                                            |
|  +---------------------------+   +----------------------------+   +----------------------------+  |
|  |        CpuContext         |   |       MemoryManager        |   |    Cranelift JIT Engine    |  |
|  | (X0-X30, SP, PC, NZCV)    |   |  (mmap virtual address space)|   | (Basic Block Compilation)  |  |
|  +---------------------------+   +----------------------------+   +----------------------------+  |
|  +---------------------------+                                    +----------------------------+  |
|  |     TargetOs Enum         |                                    |   Interpreter Fallback     |  |
|  | (Linux / Darwin / Android)|                                    |   (bad64 Instruction Step) |  |
|  +---------------------------+                                    +----------------------------+  |
+---------------------------------------------------------------------------------------------------+
                                                 |
                                 +---------------+---------------+
                                 |                               |
                                 v                               v
 +-----------------------------------------------+ +-----------------------------------------------+
 |               SyscallDispatcher               | |                maarch64-thunks                |
 |  +--------------------+ +-------------------+ | |  +-----------+ +-----------+ +--------------+ |
 |  | Linux Syscall (SVC)| | Darwin Syscall    | | |  | Host GPU  | | Host Audio| | Host LibVLC  | |
 |  | (x86_64 libc / nsys)| | (Mach Traps/BSD)  | | |  | (EGL/GLX) | |(ALSA/Pulse)| | Passthrough  | |
 |  +--------------------+ +-------------------+ | |  +-----------+ +-----------+ +--------------+ |
 +-----------------------------------------------+ |  +-----------------------+ +------------------+ |
                                                 | |  | Darwin / ObjC Bridge  | | Metal Framework  | |
                                                 | |  | (objc_msgSend Hook)   | | (MTL Device Pass)| |
                                                 | |  +-----------------------+ +------------------+ |
                                                 | +-----------------------------------------------+
                                                 |                               |
                                                 +---------------+---------------+
                                                                 |
                                                                 v
 +-------------------------------------------------------------------------------------------------+
 |                                     Host Operating System                                       |
 |                                   (Linux x86_64 / macOS)                                        |
 +-------------------------------------------------------------------------------------------------+
```

---

## ⚡ JIT アクセラレーションと対応命令セット

Cranelift JIT エンジン (`core/src/jit/`) は、AArch64 の基本ブロック（Basic Block）を動的にホスト x86_64 ネイティブコードへコンパイルします。

| 命令グループ | 対応命令 | Cranelift IR マッピング |
| :--- | :--- | :--- |
| **制御構文・分岐** | `B`, `BL`, `RET`, `CBZ`, `CBNZ` | `icmp`, `select`, 直結ジャンプ / return 文 |
| **アドレス計算** | `ADRP` | 絶対4KBページ計算 (`iconst`) |
| **算術演算** | `ADD`, `SUB` | `iadd`, `isub`（レジスタ・即値オペランド対応） |
| **ビット論理演算** | `AND`, `ORR`, `EOR`, `BIC` | `band`, `bor`, `bxor` |
| **シフト演算** | `LSL`, `LSR`, `ASR` | `ishl`, `ushr`, `sshr` |
| **データ転送** | `MOV`, `NOP` | コンテキストレジスタロード/ストア, ゼロコスト移動 |

*JIT未対応の複雑なSIMD命令やシステム命令に遭遇した場合は、レジスタ状態を崩すことなく自動的に `bad64` インタプリタへフォールバックします。*

---

## 🔌 ゼロコスト・ライブラリ・パススルー生態系 (`thunks/`)

| サブシステム | フック対象シンボル / フレームワーク | ホスト側のブリッジ先 |
| :--- | :--- | :--- |
| **🎮 GPU / 描画** | `eglGetDisplay`, `eglInitialize`, `glCreateWindowSurface`, `glDrawArrays`, `glSwapBuffers` | ホスト x86_64 `libEGL`, `libGLX`, OpenGL ES ドライバ (60 FPS) |
| **🔊 オーディオ** | `snd_pcm_open`, `snd_pcm_writei`, `pa_simple_new`, `pa_simple_write` | ホスト `libasound.so.2` (ALSA) & `libpulse.so.0` (PulseAudio) |
| **🎬 メディア** | `libvlc_new`, `libvlc_media_player_new`, `libvlc_media_player_play`, `libvlc_wait` | ホスト `libvlc.so.5` & `libvlccore.so.9` ネイティブマルチメディアエンジン |
| **🍏 macOS / Darwin** | `objc_msgSend`, `MTLCreateSystemDefaultDevice` | ホスト macOS `libobjc.A.dylib` & `Metal.framework` |

---

## 📦 ワークスペース構造

Maarch64 は `Maarch64-Project` 組織配下のモジュール式 Cargo サブモジュールで構築されています：

```
Maarch64-Project/
├── core/                # maarch64-core: ローダー, メモリ, CPUコンテキスト, システムコール, JIT
├── thunks/              # maarch64-thunks: GPU, オーディオ, VLC, Darwin, Metal パススルー
├── tools/               # maarch64-tools: maarch64-runner CLI & 差分検証ツール
├── tests/               # maarch64-tests: 統合テスト & ARM64 テスト用フィクスチャバイナリ
└── docs/                # maarch64-docs: システムアーキテクチャ仕様書および技術ドキュメント
```

---

## 📚 技術ドキュメント仕様書

- 📐 **[全体アーキテクチャ仕様書 (docs/ARCHITECTURE.md)](docs/ARCHITECTURE.md)**: 内部モジュール設計と実装詳細。
- 🌐 **[マルチOSバイナリ変換仕様 (docs/MULTI_OS.md)](docs/MULTI_OS.md)**: ELF64 vs Mach-O ARM64 ロード, System V vs Darwin ABI, システムコールルーティング。
- ⚡ **[Cranelift JIT エンジン仕様 (docs/JIT_ENGINE.md)](docs/JIT_ENGINE.md)**: コンパイルパイプライン, レジスタインデックス, 対応命令, インタプリタフォールバック。
- 🔌 **[ライブラリ・パススルー仕様 (docs/THUNKS.md)](docs/THUNKS.md)**: ホスト GPU, オーディオ, LibVLC, Metal フレームワーク パススルー構造。

---

## 🛠️ 事前準備 & ビルド手順

### 1. システム依存パッケージとクロスコンパイラのインストール (Ubuntu/Debian)

```bash
# ビルドツール & Unicorn Engine 開発用ヘッダーのインストール
sudo apt-get update
sudo apt-get install -y gcc-aarch64-linux-gnu libunicorn-dev libvlc-dev libpulse-dev libasound2-dev

# Rust ツールチェーンに AArch64 ターゲットを追加
rustup target add aarch64-unknown-linux-gnu
```

### 2. リポジトリとサブモジュールのクローン

```bash
git clone --recursive git@github.com:Maarch64-Project/Maarch64.git
cd Maarch64
```

---

## 🚀 クイックスタート & 実行方法

### 1. ワークスペース全体のビルド

```bash
cargo build --workspace
```

### 2. 統合テストの実行

```bash
cargo test --workspace
```

### 3. Unicorn オラクルとの差分検証テストの実行

```bash
cargo run -p maarch64-diff-test
```

### 4. AArch64 バイナリの実行

```bash
# インタプリタモードで実行
./target/debug/maarch64 ./sysroot/aarch64-rootfs/usr/bin/vlc

# Cranelift JIT アクセラレーション有効化で実行
./target/debug/maarch64 --jit ./sysroot/aarch64-rootfs/usr/bin/vlc

# 詳細ログ付きで実行
./target/debug/maarch64 --jit --verbose ./sysroot/aarch64-rootfs/usr/bin/vlc
```

---

## 🗺️ ロードマップと開発フェーズ

- [x] **Phase 1: コアエンジン & Linux ELF 対応** — 仮想メモリ, System V AArch64 ABI, `bad64` インタプリタ, Linux システムコールディスパッチャー。
- [x] **Phase 2: パススルー Thunk サブシステム** — OpenGL/EGL GPU パススルー, ALSA/PulseAudio パススルー, Host LibVLC パススルー。
- [x] **Phase 3: マルチOSアーキテクチャ & Mach-O 対応** — `AutoLoader`, Mach-O 64-bit ARM64 ローダー, Darwin BSD syscall & Mach Trap ルーティング。
- [x] **Phase 4: Cranelift JIT エンジン拡充** — 制御構文 (`CBZ`, `CBNZ`), `ADRP`, ビット演算 (`AND`, `EOR`), シフト (`LSL`, `LSR`, `ASR`), スタックポインタ (`SP`) インデックス。
- [ ] **Phase 5: Intel Mac 「逆 Rosetta 2」 パイプライン** — macOS x86_64 ホスト環境での 1:1 ネイティブ `.dylib` フォワーディングによる Apple Silicon アプリのネイティブ実行。

---

## 📄 ライセンス

Dual-licensed under either of:
- Apache License, Version 2.0 ([LICENSE-APACHE](LICENSE-APACHE))
- MIT License ([LICENSE-MIT](LICENSE-MIT))
