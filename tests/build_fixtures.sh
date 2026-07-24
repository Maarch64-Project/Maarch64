#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT_DIR="$SCRIPT_DIR/bin"
mkdir -p "$OUT_DIR"

echo "[+] Building AArch64 test binary fixtures..."

# 1. Minimal Assembly Hello World
ASM_SRC="$SCRIPT_DIR/fixtures/asm/hello_asm.s"
OUT_BIN="$OUT_DIR/hello_arm64"

CC="aarch64-linux-gnu-gcc"

if command -v $CC &> /dev/null; then
    $CC -nostdlib -static "$ASM_SRC" -o "$OUT_BIN"
    echo "[+] Successfully compiled $OUT_BIN using $CC"
elif command -v clang &> /dev/null; then
    clang --target=aarch64-linux-gnu -nostdlib -static "$ASM_SRC" -o "$OUT_BIN"
    echo "[+] Successfully compiled $OUT_BIN using clang"
else
    echo "[-] Warning: No AArch64 cross compiler found (aarch64-linux-gnu-gcc or clang)."
fi
