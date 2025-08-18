#!/bin/bash
set -e

echo "[*] Building application"

# Create directory for build output
mkdir -p build_execs

# Compile source files to object files

gcc -m32 -ffreestanding -fno-pic -fno-plt -fno-pie -c sys_executables/console/main.c -o build_execs/main.o
gcc -m32 -ffreestanding -fno-pic -fno-plt -fno-pie -c sys_executables/console/Graphics/graphics.c -o build_execs/graphics.o
gcc -m32 -ffreestanding -fno-pic -fno-plt -fno-pie -c sys_executables/console/res.c -o build_execs/res.o


# Create relocatable ELF with relocation info preserved
ld -m elf_i386 -r -o build_execs/console.rel build_execs/main.o build_execs/graphics.o build_execs/res.o build_lib/libsys.a


# Link relocatable ELF at fixed address to produce final executable ELF
ld -m elf_i386 -nostdlib -Ttext=0x200000 -e app_main -o build_execs/console.elf build_execs/console.rel

# Extract flat binary from final ELF
objcopy -O binary build_execs/console.elf build_execs/console.bin


rm -f build_execs/*.o
readelf -r build_execs/console.rel > build_execs/console_rel.txt
rm -f build_execs/*.rel
rm -f build_execs/*.elf

echo "[*] build complete:"
echo "    Relocatable ELF (with reloc info): build_execs/console.rel"
echo "    Linked ELF executable: build_execs/console.elf"
echo "    Flat binary (raw loadable): build_execs/console.bin"

