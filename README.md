# Maarch64

**Maarch64** is a high-performance, user-space AArch64 (ARM64) binary translator / emulator designed to execute AArch64 Linux binaries natively on x86_64 host environments.

Unlike full-system emulators such as QEMU, Maarch64 prioritizes near-native execution speed by utilizing **Library Wrapping (Thunking)**—directly intercepting shared library calls (`glibc`, `libX11`, `OpenGL`, etc.) and redirecting them to the host x86_64 system libraries.

---

## 🏗️ Repository Architecture

Maarch64 is structured as a modular Rust Cargo Workspace using Git submodules under the `Maarch64-Project` GitHub organization:

| Submodule | Repository | Description |
| :--- | :--- | :--- |
| **`core`** | [`maarch64-core`](https://github.com/Maarch64-Project/maarch64-core) | Core engine: CPU state (`CpuContext`), `mmap`-backed virtual memory (`MemoryManager`), ELF64 loader (`ElfLoader`), AArch64 instruction decoder/interpreter, and Linux system call dispatcher (`SyscallDispatcher`). |
| **`thunks`** | [`maarch64-thunks`](https://github.com/Maarch64-Project/maarch64-thunks) | Library wrapping & dynamic PLT/GOT symbol interception framework. |
| **`tools`** | [`maarch64-tools`](https://github.com/Maarch64-Project/maarch64-tools) | CLI tools: `maarch64-runner` (binary executor) and `maarch64-diff-test` (Unicorn Engine oracle differential test harness). |
| **`tests`** | [`maarch64-tests`](https://github.com/Maarch64-Project/maarch64-tests) | AArch64 test fixtures (`asm/`, `c/`, `rust/`), cross-compilation build script (`build_fixtures.sh`), and integration unit test suite. |
| **`docs`** | [`maarch64-docs`](https://github.com/Maarch64-Project/maarch64-docs) | Architecture specification and roadmap documentation. |

---

## 🛠️ Prerequisites

Install required build dependencies and cross-compilers (Ubuntu/Debian):

```bash
# 1. Install system packages & cross-compilers
sudo apt-get update
sudo apt-get install -y gcc-aarch64-linux-gnu libunicorn-dev

# 2. Add AArch64 target to Rust toolchain
rustup target add aarch64-unknown-linux-gnu
```

---

## 🚀 Quick Start & Development Workflow

### 1. Clone Repository with Submodules

```bash
git clone --recursive git@github.com:Maarch64-Project/Maarch64.git
cd Maarch64
```

### 2. Build AArch64 Test Binary Fixtures

Cross-compile all test fixtures (`asm/*.s`, `c/*.c`, `rust/*.rs`) into `tests/bin/`:

```bash
bash tests/build_fixtures.sh
```

### 3. Run Unit & Integration Tests

Run the full Rust workspace test suite:

```bash
cargo test --workspace
```

### 4. Run Unicorn Engine Differential State Verification

Execute step-by-step bit-exact register verification against the Unicorn Engine oracle:

```bash
cargo run -p maarch64-diff-test
```

### 5. Execute an ARM64 Binary via Maarch64 Runner

Execute a cross-compiled AArch64 binary using the emulated translator:

```bash
# Assembly test target
cargo run -p maarch64-runner -- tests/bin/hello_asm

# Rust test target
cargo run -p maarch64-runner -- tests/bin/hello_rust
```

---

## 🌿 Branch & Contribution Strategy

- **`main`**: Production & stable release branch.
- **`dev`**: Active development and integration branch.
- **Feature Development Flow**:
  1. Create a feature branch from `dev`: `git checkout dev && git checkout -b feature/issue-#-name`
  2. Implement changes and write unit tests in `tests/`
  3. Submit a Pull Request targeting `dev` for review and automated CI checks.

---

## 📄 License

Dual-licensed under either of:
- Apache License, Version 2.0 ([LICENSE-APACHE](LICENSE-APACHE))
- MIT License ([LICENSE-MIT](LICENSE-MIT))
