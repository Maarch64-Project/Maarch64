# 🚀 Maarch64: Multi-OS AArch64 Dynamic Binary Translator

<div align="center">

[![Rust](https://img.shields.io/badge/Rust-2021%20Edition-orange.svg)](https://www.rust-lang.org/)
[![Architecture](https://img.shields.io/badge/Architecture-AArch64%20%E2%86%92%20x86__64-blue.svg)]()
[![Target OS](https://img.shields.io/badge/Target%20OS-Linux%20%7C%20macOS%20Darwin-green.svg)]()
[![JIT Engine](https://img.shields.io/badge/JIT%20Engine-Cranelift-purple.svg)](https://cranelift.dev/)
[![License](https://img.shields.io/badge/License-MIT%20%2F%20Apache--2.0-red.svg)]()

**Maarch64** is a high-performance, platform-agnostic AArch64 (ARM 64-bit) user-space dynamic binary translator written in Rust. It executes AArch64 binaries—from both **Linux (ELF64)** and **macOS (Mach-O ARM64)**—natively on x86_64 host environments with near-native throughput.

</div>

---

## 🌟 Key Innovations & Value Proposition

Traditional full-system emulators (such as QEMU) execute user-space binaries by interpreting or JIT-compiling every single library call inside the emulated CPU context, causing severe rendering lag and CPU bottlenecks.

Maarch64 solves this by pioneering **Zero-Cost Library Passthrough (Thunking)** coupled with a **Hybrid Cranelift JIT Engine**:

```
+---------------------------------------------------------------------------------------------------+
|                                 Maarch64 Execution Philosophy                                     |
|                                                                                                   |
|   Target ARM64 App Logic ------------[ Cranelift JIT Engine ]-----------> High-Speed Execution    |
|   Shared Library Calls (GL/VLC/Audio) -[ Zero-Cost Thunking Bridge ]---> Native Host x86_64 Drivers |
+---------------------------------------------------------------------------------------------------+
```

- **⚡ Near-Native Speed**: System libraries (`glibc`, `libX11`, `OpenGL`, `ALSA`, `PulseAudio`, `LibVLC`, `Metal`) bypass emulation completely and execute directly on host x86_64 hardware.
- **🌐 Multi-OS Binary Loading**: Auto-detects and loads both Linux ELF64 and macOS Mach-O 64-bit binaries dynamically via `AutoLoader`.
- **🍏 Reverse Rosetta 2 Vision**: Designed to execute Apple Silicon ARM64 binaries on Intel Mac (macOS x86_64) hosts with 1:1 host `.dylib` symbol forwarding.
- **🔬 Bit-Exact Verification**: Includes a differential testing harness (`maarch64-diff-test`) powered by the Unicorn Engine oracle.

---

## 🏗️ High-Level System Architecture

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

## ⚡ JIT Acceleration & Instruction Set Support

The Cranelift JIT engine (`core/src/jit/`) translates AArch64 basic blocks dynamically into native host machine code.

| Opcode Group | Supported Instructions | Cranelift IR Mapping |
| :--- | :--- | :--- |
| **Control Flow** | `B`, `BL`, `RET`, `CBZ`, `CBNZ` | `icmp`, `select`, direct jumps & return statements |
| **Address Calculation** | `ADRP` | Absolute 4KB page calculation (`iconst`) |
| **Arithmetic** | `ADD`, `SUB` | `iadd`, `isub` with register & immediate operands |
| **Bitwise Operations** | `AND`, `ORR`, `EOR`, `BIC` | `band`, `bor`, `bxor` |
| **Shift Operations** | `LSL`, `LSR`, `ASR` | `ishl`, `ushr`, `sshr` |
| **Data Movement** | `MOV`, `NOP` | Context register load/store, zero-cost pass |

*Non-JIT'd or complex SIMD/system instructions automatically fall back to the `bad64` interpreter without desynchronizing register state.*

---

## 🔌 Zero-Cost Passthrough Ecosystem (`thunks/`)

| Subsystem | Intercepted Symbols / Frameworks | Host Target Bridge |
| :--- | :--- | :--- |
| **🎮 GPU / Display** | `eglGetDisplay`, `eglInitialize`, `glCreateWindowSurface`, `glDrawArrays`, `glSwapBuffers` | Host x86_64 `libEGL`, `libGLX`, OpenGL ES drivers (60 FPS) |
| **🔊 Audio** | `snd_pcm_open`, `snd_pcm_writei`, `pa_simple_new`, `pa_simple_write` | Host `libasound.so.2` (ALSA) & `libpulse.so.0` (PulseAudio) |
| **🎬 Media Engine** | `libvlc_new`, `libvlc_media_player_new`, `libvlc_media_player_play`, `libvlc_wait` | Host `libvlc.so.5` & `libvlccore.so.9` native multimedia engine |
| **🍏 macOS / Darwin** | `objc_msgSend`, `MTLCreateSystemDefaultDevice` | Host macOS `libobjc.A.dylib` & `Metal.framework` |

---

## 📦 Workspace Repository Structure

Maarch64 is organized into modular Cargo submodules under the `Maarch64-Project` organization:

```
Maarch64-Project/
├── core/                # maarch64-core: Loader, Memory, CPU Context, Syscalls, JIT
├── thunks/              # maarch64-thunks: GPU, Audio, VLC, Darwin, Metal Passthrough
├── tools/               # maarch64-tools: maarch64-runner CLI & differential test harness
├── tests/               # maarch64-tests: Integration tests & ARM64 fixture binaries
└── docs/                # maarch64-docs: System architecture and technical specifications
```

---

## 📚 Technical Documentation

- 📐 **[Architecture Specification](docs/ARCHITECTURE.md)**: Deep-dive into internal crate components and design choices.
- 🌐 **[Multi-OS Binary Translation](docs/MULTI_OS.md)**: ELF64 vs Mach-O ARM64 loading, System V vs Darwin ABI, and syscall dispatching.
- ⚡ **[Cranelift JIT Engine](docs/JIT_ENGINE.md)**: Compilation pipeline, register indexing, supported instructions, and interpreter fallback.
- 🔌 **[Library Passthrough Thunking](docs/THUNKS.md)**: Host GPU, Audio, LibVLC, and Metal framework passthrough architecture.

---

## 🛠️ Prerequisites & Installation

### 1. Install System Dependencies & Cross-Compilers (Ubuntu/Debian)

```bash
# Install build tools & Unicorn Engine development headers
sudo apt-get update
sudo apt-get install -y gcc-aarch64-linux-gnu libunicorn-dev libvlc-dev libpulse-dev libasound2-dev

# Add AArch64 target to Rust toolchain
rustup target add aarch64-unknown-linux-gnu
```

### 2. Clone Workspace with Submodules

```bash
git clone --recursive git@github.com:Maarch64-Project/Maarch64.git
cd Maarch64
```

---

## 🚀 Quick Start & Usage

### 1. Build Full Workspace

```bash
cargo build --workspace
```

### 2. Run Integration Tests

```bash
cargo test --workspace
```

### 3. Run Differential State Verification (vs Unicorn Oracle)

```bash
cargo run -p maarch64-diff-test
```

### 4. Execute AArch64 Binaries

```bash
# Execute in Interpreter mode
./target/debug/maarch64 ./sysroot/aarch64-rootfs/usr/bin/vlc

# Execute with Cranelift JIT Acceleration
./target/debug/maarch64 --jit ./sysroot/aarch64-rootfs/usr/bin/vlc

# Execute with Verbose Execution Logging
./target/debug/maarch64 --jit --verbose ./sysroot/aarch64-rootfs/usr/bin/vlc
```

---

## 🗺️ Roadmap & Development Phases

- [x] **Phase 1: Core Engine & Linux ELF Support** — Virtual memory, System V AArch64 ABI, `bad64` interpreter, Linux syscall dispatcher.
- [x] **Phase 2: Passthrough Thunk Subsystems** — OpenGL/EGL GPU passthrough, ALSA/PulseAudio passthrough, Host LibVLC multimedia passthrough.
- [x] **Phase 3: Multi-OS Architecture & Mach-O Support** — `AutoLoader`, Mach-O 64-bit ARM64 loader, Darwin BSD syscall & Mach Trap routing.
- [x] **Phase 4: Cranelift JIT Engine Expansion** — Control flow (`CBZ`, `CBNZ`), `ADRP`, bitwise (`AND`, `EOR`), shifts (`LSL`, `LSR`, `ASR`), and stack pointer (`SP`) indexing.
- [ ] **Phase 5: Intel Mac "Reverse Rosetta 2" Pipeline** — Direct host `.dylib` symbol forwarding on macOS x86_64 host machines for native Apple Silicon app execution.

---

## 📄 License

Dual-licensed under either of:
- Apache License, Version 2.0 ([LICENSE-APACHE](LICENSE-APACHE))
- MIT License ([LICENSE-MIT](LICENSE-MIT))
