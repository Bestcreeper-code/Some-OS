#!/bin/bash
set -e

echo "[*] Building Notepad application (Freestanding with custom OS library, Entry: app_main)..."

# Create directory for the build output
mkdir -p build_execs

# Compile the source with -ffreestanding (no libc)
gcc -m32 -ffreestanding  -fno-pic -fno-pie -c sys_executables/notepad/notepad_main.c -o build_execs/notepad_main.o

ld -m elf_i386 -nostdlib -Ttext=0x200000 --oformat binary -e app_main \
  -o build_execs/notepad.bin build_execs/notepad_main.o build_lib/libsys.a

# Clean up object files
rm -f build_execs/*.o

echo "[*] Notepad build complete: build_execs/notepad.bin"
