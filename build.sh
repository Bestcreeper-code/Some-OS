#!/bin/bash
set -e  # Stop on error

echo "[0] Preparing build directories..."
mkdir -p build iso/boot/grub

# === Assemble multiboot header ===
echo "[1] Assembling multiboot header..."
nasm -f elf32 src/multiboot_header.asm -o build/multiboot_header.o

# === Assemble irq0 handler ===
echo "[1.1] Assembling irq0 handler..."
nasm -f elf32 src/asm/irq0_handle.asm -o build/irq0_handle.o
nasm -f elf32 src/asm/irq12_handle.asm -o build/irq12_handle.o
nasm -f elf32 src/asm/isr13.asm -o build/isr13.o
nasm -f elf32 src/asm/dummy_handle.asm -o build/dummy_handle.o
nasm -f elf32 src/asm/gdt.asm -o build/gdt_asm.o
nasm -f elf32 src/asm/keyboard_interrupt.asm -o build/keyboard_interrupt_asm.o
nasm -f elf32 src/asm/processes.asm -o build/processes_asm.o

# === Compile kernel and core files ===
echo "[2] Compiling kernel and core files..."
gcc -m32 -g -ffreestanding -c src/kernel.c -o build/kernel.o
gcc -m32 -g -ffreestanding -c src/console.c -o build/console.o
gcc -m32 -g -ffreestanding -c src/memory.c -o build/memory.o
gcc -m32 -g -ffreestanding -c src/time.c -o build/time.o
gcc -m32 -g -ffreestanding -c src/io.c -o build/io.o
gcc -m32 -g -ffreestanding -c src/idt.c -o build/idt.o
gcc -m32 -g -ffreestanding -c src/FileSystem.c -o build/FileSystem.o
gcc -m32 -g -ffreestanding -c src/string.c -o build/string.o
gcc -m32 -g -ffreestanding -c src/ATA_IO.c -o build/ATA_IO.o
gcc -m32 -g -ffreestanding -c src/multiboot_info.c -o build/multiboot_info.o
gcc -m32 -g -ffreestanding -c src/random.c -o build/random.o
gcc -m32 -g -ffreestanding -c src/gdt.c -o build/gdt.o
gcc -m32 -g -ffreestanding -c src/video.c -o build/video.o
gcc -m32 -g -ffreestanding -c src/data/textconsts.c -o build/textconsts.o
gcc -m32 -g -ffreestanding -c src/math.c -o build/math.o
gcc -m32 -g -ffreestanding -c src/vga_modes.c -o build/vga_modes.o
gcc -m32 -g -ffreestanding -c src/loader.c -o build/loader.o
gcc -m32 -g -ffreestanding -c src/mouse.c -o build/mouse.o

# === Compile FatFs ===
echo "[3] Compiling FatFs..."
gcc -m32 -g -ffreestanding -c FatFs/ff.c -o build/ff.o
gcc -m32 -g -ffreestanding -c FatFs/diskio.c -o build/diskio.o
gcc -m32 -g -ffreestanding -c FatFs/ffsystem.c -o build/ffsystem.o
gcc -m32 -g -ffreestanding -c FatFs/ffunicode.c -o build/ffunicode.o

# === Link kernel ===
echo "[4] Linking kernel ELF..."
ld -m elf_i386 -Ttext=0x100000 -z noexecstack -o kernel.elf \
  build/multiboot_header.o \
  build/irq0_handle.o build/keyboard_interrupt_asm.o \
  build/kernel.o build/console.o build/memory.o build/random.o \
  build/time.o build/io.o build/string.o \
  build/ATA_IO.o build/FileSystem.o build/multiboot_info.o \
  build/ff.o build/diskio.o build/ffsystem.o build/ffunicode.o \
  build/idt.o build/isr13.o build/dummy_handle.o build/gdt.o build/gdt_asm.o \
  build/video.o build/textconsts.o build/vga_modes.o build/loader.o \
  build/processes_asm.o build/mouse.o build/irq12_handle.o


# === Convert to binary for GRUB ===
echo "[5] Generating kernel.bin..."
objcopy -O binary kernel.elf iso/boot/kernel.bin

# === Copy kernel ELF to ISO ===
echo "[6] Copying kernel.elf to ISO..."
cp kernel.elf iso/boot/kernel.elf

# === Write GRUB config ===
echo "[7] Writing GRUB config..."
cat > iso/boot/grub/grub.cfg << EOF
set timeout=0
set default=0
menuentry "My OS" {
    multiboot /boot/kernel.elf
    boot
}
EOF

# === Build bootable ISO ===
echo "[8] Creating os.iso with GRUB..."
grub-mkrescue -o os.iso iso/

# === Create and format FAT32 disk image if not present ===
if [ ! -f disk.img ]; then
  echo "[9] Creating and formatting disk.img (FAT32, 512MB)..."
  dd if=/dev/zero of=disk.img bs=1M count=512
  sync
  mkfs.fat -F 32 disk.img
  sync
  echo "[*] Optionally: use mcopy to add files: mcopy -i disk.img file.txt ::file.txt"
else
  echo "[*] disk.img already exists, skipping creation."
fi

# === Launch QEMU ===
if [ "$1" = "-d" ]; then
  echo "[10] Launching QEMU in GDB mode..."
  qemu-system-i386 -boot d -cdrom os.iso -m 512M -drive file=disk.img,format=raw,if=ide -s -S #-no-reboot -d int,cpu_reset
else
  echo "[10] Launching QEMU..."
  qemu-system-i386 -boot d -cdrom os.iso -m 512M -drive file=disk.img,format=raw,if=ide #-s -S #-no-reboot -d int,cpu_reset
fi



echo "[*] Build complete!"

