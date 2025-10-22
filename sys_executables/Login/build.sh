#!/bin/bash
set -e

echo "[*] Building application"

mkdir -p build_execs

echo "[*] Compiling source files..."
gcc -g -m32 -ffreestanding -fno-pic -fno-pie -c sys_executables/Login/Login.c -o build_execs/Login.o
gcc -g -m32 -ffreestanding -fno-pic -fno-pie -c sys_executables/Login/Graphics/graphics.c -o build_execs/graphics.o
gcc -g -m32 -ffreestanding -fno-pic -fno-pie -c sys_executables/Login/res.c -o build_execs/res.o

echo "[*] Linking into ET_EXEC ELF with high load address..."
ld -m elf_i386 -nostdlib -static -Ttext=0x4000000 -e main -o build_execs/Login.elf \
    build_execs/Login.o \
    build_execs/graphics.o \
    build_execs/res.o \
    build_lib/libsys.a
    # custom_libc/crt0.o \

echo "[*] Build complete:"
echo "    Executable ELF: build_execs/Login.elf "

echo "[*] Cleaning up intermediate object files..."
rm -f build_execs/*.o
