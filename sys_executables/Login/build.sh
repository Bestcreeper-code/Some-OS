#!/bin/bash
set -e

echo "[*] Building application"

mkdir -p build_execs

# Compile source files to object files
gcc -g -m32 -fno-pic -ffreestanding -nostdlib -c sys_executables/Login/Login.c -o build_execs/Login.o
gcc -g -m32 -fno-pic -ffreestanding -nostdlib -c sys_executables/Login/Graphics/graphics.c -o build_execs/graphics.o
gcc -g -m32 -fno-pic -ffreestanding -nostdlib -c sys_executables/Login/res.c -o build_execs/res.o

# Create relocatable ELF (.rel) with relocation info preserved
ld -m elf_i386 -r -o build_execs/Login.elf build_execs/Login.o build_execs/graphics.o build_execs/res.o build_lib/libsys.a

echo "[*] Build complete:"
echo "    Relocatable ELF: build_execs/Login.rel"
echo "    Linked ELF: build_execs/Login.elf"
echo "    Flat binary: build_execs/login.bin"

# Cleanup intermediate object files
rm -f build_execs/*.o
# Keep .rel and .elf for inspection
