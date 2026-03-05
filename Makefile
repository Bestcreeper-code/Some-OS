#!/usr/bin/make -f

# === Tools ===
CC = gcc
NASM = nasm
LD = ld
OBJCOPY = objcopy

ARCH = x86
ARCH_DIR = src/arch/$(ARCH)


INCLUDE_DIRS = src src/config src/headers \
	src/bootloader                        \
	src/arch/includes $(ARCH_DIR) $(ARCH_DIR)/elf  \
	$(ARCH_DIR)/memory $(ARCH_DIR)/init   \
	$(ARCH_DIR)/asm $(ARCH_DIR)/cpu 	  \
	$(ARCH_DIR)/scheduler $(ARCH_DIR)/panic\
	\
	\
	\
	\
	src/drivers/ATA src/drivers/PS-2 

INCLUDES := $(addprefix -I,$(INCLUDE_DIRS))

CFLAGS = -m32 -g -ffreestanding $(INCLUDES) -fno-stack-protector -mno-sse -mno-sse2 -fno-tree-vectorize

LDFLAGS = -m elf_i386 -T src/arch/$(ARCH)/linker.ld -z noexecstack 

# === Directories ===
SRC_DIRS = src FatFs 
BUILD_DIR = build
ISO_DIR = iso/boot
GRUB_DIR = $(ISO_DIR)/grub

# === Output files ===
KERNEL_ELF = $(ISO_DIR)/kernel.elf
KERNEL_BIN = $(ISO_DIR)/kernel.bin
DISK_IMG = disk.img
ISO_FILE = os.iso

# === Source discovery ===
C_SOURCES := $(shell find $(SRC_DIRS) -type f -name "*.c")
ASM_SOURCES := $(shell find $(SRC_DIRS) -type f -name "*.asm")

C_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst %.asm,$(BUILD_DIR)/%_asm.o,$(ASM_SOURCES))

MULTIBOOT_OBJ := $(BUILD_DIR)/src/multiboot_header.o
FILTERED_ASM_OBJECTS := $(filter-out $(MULTIBOOT_OBJ),$(ASM_OBJECTS))
OBJECTS := $(C_OBJECTS) $(FILTERED_ASM_OBJECTS)

SYMS_BIN = syms.bin
SYMS_OBJ = $(BUILD_DIR)/syms.o

# === Targets ===
.PHONY: all run gdb clean disk-img iso

all: $(KERNEL_BIN) $(KERNEL_ELF) iso

# === Compilation rules ===
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%_asm.o: %.asm
	@mkdir -p $(dir $@)
	@echo "Assembling $<"
	$(NASM) -f elf32 $< -o $@

# Special rule for multiboot header
$(MULTIBOOT_OBJ): src/bootloader/multiboot1/multiboot_header.asm
	@mkdir -p $(dir $@)
	@echo "Assembling multiboot header..."
	$(NASM) -f elf32 $< -o $@

# Convert syms.bin to ELF object
$(SYMS_OBJ): $(SYMS_BIN)
	@mkdir -p $(dir $@)
	@echo "Converting syms.bin to object..."
	objcopy -I binary -O elf32-i386 -B i386 $(SYMS_BIN) $(SYMS_OBJ)

# Link kernel ELF (syms.o must come first!)
$(KERNEL_ELF): $(SYMS_OBJ) $(MULTIBOOT_OBJ) $(OBJECTS) | $(ISO_DIR)
	@echo "Linking kernel ELF..."
	$(LD) $(LDFLAGS) -o $@ $(SYMS_OBJ) $(MULTIBOOT_OBJ) $(OBJECTS)
	cp $(KERNEL_ELF) ./

# Generate flat binary for GRUB
$(KERNEL_BIN): $(KERNEL_ELF)
	@echo "Generating kernel.bin..."
	$(OBJCOPY) -O binary $< $@

# Directories
$(ISO_DIR):
	mkdir -p $(GRUB_DIR)

# === ISO ===
iso: $(KERNEL_ELF) $(KERNEL_BIN) $(GRUB_DIR)
	
	sh ./syms_file_maker.sh $(KERNEL_ELF) $(SYMS_BIN)
	
	objcopy -I binary -O elf32-i386 -B i386 $(SYMS_BIN) $(SYMS_OBJ)
	
	$(LD) $(LDFLAGS) -o $(KERNEL_ELF) $(SYMS_OBJ) $(MULTIBOOT_OBJ) $(OBJECTS)
	
	cp $(KERNEL_ELF) ./

	@echo "Creating GRUB bootable ISO..."
	grub-mkrescue -o $(ISO_FILE) iso/

# === Disk image ===
disk-img:
	@if [ ! -f $(DISK_IMG) ]; then \
		echo "Creating FAT32 disk image..."; \
		dd if=/dev/zero of=$(DISK_IMG) bs=1M count=512; \
		mkfs.fat -F 32 $(DISK_IMG); \
	else \
		echo "$(DISK_IMG) already exists."; \
	fi

# === Run QEMU ===
run: all
	qemu-system-i386 \
		-m 512M \
		-boot d \
		-cdrom $(ISO_FILE) \
		-drive file=$(DISK_IMG),format=raw,if=ide \
		-usb \
		-device VGA,vgamem_mb=32 \
		-display sdl \
		-serial stdio

# 		-device usb-storage,drive=usbdisk \
# 		-drive file=usb.img,if=none,id=usbdisk,format=raw \

# === Run with GDB ===
gdb: all
	qemu-system-i386 \
		-m 512M \
		-boot d \
		-cdrom $(ISO_FILE) \
		-drive file=$(DISK_IMG),format=raw,if=ide \
		-serial stdio \
		-s -S \
		-display gtk 

noreboot: all
	qemu-system-i386 \
		-m 512M \
		-boot d \
		-cdrom $(ISO_FILE) \
		-drive file=$(DISK_IMG),format=raw,if=ide \
		-serial stdio \
		-s -S \
		-display gtk \
		-no-reboot \
		-d int,cpu_reset,unimp,guest_errors \
		-D qemu-emulogs.txt

# === Clean ===
clean:
	rm -rf $(BUILD_DIR) $(ISO_FILE) $(KERNEL_ELF) $(KERNEL_BIN)
