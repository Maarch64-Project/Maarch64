# Maarch64

**Maarch64** is a high-performance, platform-agnostic AArch64 (ARM 64-bit) user-space binary translator designed to execute AArch64 binaries—from both **Linux (ELF64)** and **macOS (Mach-O ARM64)**—natively on x86_64 host environments with near-native throughput.

Unlike traditional full-system emulators (such as QEMU), Maarch64 prioritizes native execution speed by combining a **hybrid Cranelift JIT engine** with **Zero-Cost Library Passthrough Thunking**—directly intercepting shared library calls (`glibc`, `libX11`, `libEGL`, `OpenGL`, `ALSA`, `PulseAudio`, `LibVLC`, `Metal`) and delegating execution directly to the host x86_64 system libraries.

---

## 🏗️ Repository & Module Architecture

Maarch64 is structured as a modular Rust Cargo Workspace using Git submodules under the `Maarch64-Project` GitHub organization:

| Submodule | Repository | Description |
| :--- | :--- | :--- |
| **`core`** | [`maarch64-core`](https://github.com/Maarch64-Project/maarch64-core) | Core engine: `CpuContext`, `MemoryManager`, `AutoLoader` (ELF64 & Mach-O ARM64), `SyscallDispatcher` (Linux & Darwin BSD/Mach Traps), Cranelift JIT compiler, and `bad64` interpreter fallback. |
| **`thunks`** | [`maarch64-thunks`](https://github.com/Maarch64-Project/maarch64-thunks) | Passthrough library wrapping registry (`gpu`, `audio`, `vlc`, `darwin` ObjC, `metal`). |
| **`tools`** | [`maarch64-tools`](https://github.com/Maarch64-Project/maarch64-tools) | CLI tools: `maarch64-runner` (binary executor) and `maarch64-diff-test` (Unicorn Engine oracle differential test harness). |
| **`tests`** | [`maarch64-tests`](https://github.com/Maarch64-Project/maarch64-tests) | AArch64 test fixtures (`asm/`, `c/`, `rust/`), cross-compilation build scripts, and integration test suite. |
| **`docs`** | [`maarch64-docs`](https://github.com/Maarch64-Project/maarch64-docs) | Architecture specifications and documentation. |

---

## 📚 Technical Documentation

- 📐 **[Architecture Overview](docs/ARCHITECTURE.md)**: High-level system diagram and crate layout.
- 🌐 **[Multi-OS Binary Translation](docs/MULTI_OS.md)**: ELF64 vs Mach-O ARM64 loading, System V vs Darwin ABI, Linux & Darwin syscall dispatching.
- ⚡ **[Cranelift JIT Engine](docs/JIT_ENGINE.md)**: Dynamic translation pipeline, register mapping, supported opcodes, and interpreter fallback.
- 🔌 **[Library Passthrough Thunking](docs/THUNKS.md)**: Host GPU (EGL/GLX), Audio (ALSA/Pulse), LibVLC, and Metal framework passthrough architecture.

---

## 🛠️ Prerequisites & Setup

Install required build dependencies and cross-compilers (Ubuntu/Debian):

```bash
# 1. Install system packages & cross-compilers
sudo apt-get update
sudo apt-get install -y gcc-aarch64-linux-gnu libunicorn-dev

# 2. Add AArch64 target to Rust toolchain
rustup target add aarch64-unknown-linux-gnu
```

---

## 🚀 Quick Start & Execution

### 1. Clone Repository with Submodules

```bash
git clone --recursive git@github.com:Maarch64-Project/Maarch64.git
cd Maarch64
```

### 2. Build Workspace

```bash
cargo build --workspace
```

### 3. Execute ARM64 Binaries

```bash
# Execute in interpreter mode
./target/debug/maarch64 ./sysroot/aarch64-rootfs/usr/bin/vlc

# Execute with Cranelift JIT acceleration
./target/debug/maarch64 --jit ./sysroot/aarch64-rootfs/usr/bin/vlc
```

---

## 📄 License

Dual-licensed under either of:
- Apache License, Version 2.0 ([LICENSE-APACHE](LICENSE-APACHE))
- MIT License ([LICENSE-MIT](LICENSE-MIT))
