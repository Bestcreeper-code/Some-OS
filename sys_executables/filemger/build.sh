#!/bin/bash
set -e

echo "[*] Building application"

# mkdir -p build_execs

echo "[*] Compiling source files..."
gcc -g -m32 -ffreestanding -fno-pic -fno-pie -c sys_executables/filemger/filemger.c -o build_execs/filemger.o
gcc -g -m32 -ffreestanding -fno-pic -fno-pie -c sys_executables/filemger/Graphics/graphics.c -o build_execs/graphics.o
gcc -g -m32 -ffreestanding -fno-pic -fno-pie -c sys_executables/filemger/res.c -o build_execs/res.o


echo "[*] Creating fully relocatable ELF (.rel) with CRT..."
ld -m elf_i386 -r -nostdlib -o build_execs/filemger.rel \
    custom_libc/crt0.o \
    build_execs/filemger.o \
    build_execs/graphics.o \
    build_execs/res.o \
    build_lib/libsys.a

echo "[*] Build complete:"
echo "    Relocatable ELF: build_execs/filemger.rel"

# Optional: produce a fully linked ELF with entry point if needed
# ld -m elf_i386 -nostdlib -e _start -o build_execs/filemger.elf build_execs/filemger.rel build_lib/libsys.a

echo "[*] Cleaning up intermediate object files..."
rm -f build_execs/*.o
