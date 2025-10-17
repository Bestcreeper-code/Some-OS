#!/bin/bash
set -e

echo "[0] Preparing build_lib directory..."
mkdir -p build_lib

# === Assemble ASM files ===
echo "[1] Assembling ASM files in src/asm..."
for asm_file in src/asm/*.asm; do
  obj_file="build_lib/$(basename "${asm_file%.asm}_asm.o")"
  echo "  Assembling $asm_file -> $obj_file"
  nasm -f elf32 "$asm_file" -o "$obj_file"
done

# === Compile C source files in src/... ===
echo "[2.1] Compiling C source files in src/..."
for src_file in src/*.c; do
  obj_file="build_lib/$(basename "${src_file%.c}.o")"
  echo "  Compiling $src_file -> $obj_file"
  gcc -m32 -Os -ffreestanding -fno-pic -fno-pie -c "$src_file" -o "$obj_file"
done

# Compile C files in src/data/
echo "[2.2] Compiling C source files in src/data/..."
for src_file in src/data/*.c; do
  obj_file="build_lib/$(basename "${src_file%.c}.o")"
  echo "  Compiling $src_file -> $obj_file"
  gcc -m32 -Os -ffreestanding -fno-pic -fno-pie -c "$src_file" -o "$obj_file"
done

# Compile C files in FatFs/
echo "[2.3] Compiling C source files in FatFs/..."
for src_file in FatFs/*.c; do
  obj_file="build_lib/$(basename "${src_file%.c}.o")"
  echo "  Compiling $src_file -> $obj_file"
  gcc -m32 -Os -ffreestanding -fno-pic -fno-pie -c "$src_file" -o "$obj_file"
done


# === Create static library ===
echo "[3] Creating static library build_lib/libsys.a ..."
ar rcs build_lib/libsys.a build_lib/*.o

# Make sure the index is created (some systems need ranlib explicitly)
ranlib build_lib/libsys.a

echo "[*] build_lib complete!"
