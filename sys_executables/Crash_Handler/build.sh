#!/bin/bash
set -e

echo "[*] Building application"

# Create directory for build output
mkdir -p build_execs

# Compile source files to object files

gcc -g -m32 -ffreestanding -fno-pic -fno-plt -fno-pie -c sys_executables/Crash_handler/Crash_handler.c -o build_execs/Crash_handler.o



# Create relocatable ELF with relocation info preserveds
ld -m elf_i386 -r -o build_execs/crashhndl.rel build_execs/Crash_handler.o build_lib/libsys.a


# Link relocatable ELF at fixed address to produce final executable ELF
# ld -m elf_i386 -nostdlib -Ttext=0x200000 -e app_main -o build_execs/Crash_handler.elf build_execs/Crash_handler.rel

# Extract flat binary from final ELF
# objcopy -O binary build_execs/Crash_handler.elf build_execs/crashhndl.bin


rm -f build_execs/*.o
# readelf -r build_execs/Crash_handler.rel > build_execs/Crash_handler_rel.txt
# rm -f build_execs/*.rel
# rm -f build_execs/*.elf

echo "[*] build complete:"
echo "    Relocatable ELF (with reloc info): build_execs/Crash_handler.rel"
echo "    Linked ELF executable: build_execs/Crash_handler.elf"
echo "    Flat binary (raw loadable): build_execs/Crash_handler.bin"