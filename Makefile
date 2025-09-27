# === Configuration ===
CC = gcc
NASM = nasm
LD = ld
OBJCOPY = objcopy
CFLAGS = -m32 -g -ffreestanding
LDFLAGS = -m elf_i386 -Ttext=0x100000 -z noexecstack

SRC_DIRS = src FatFs
BUILD_DIR = build
ISO_DIR = iso/boot
GRUB_DIR = iso/boot/grub

KERNEL_ELF = $(ISO_DIR)/kernel.elf
KERNEL_BIN = $(ISO_DIR)/kernel.bin
DISK_IMG = disk.img
ISO_FILE = os.iso

# === File discovery ===
C_SOURCES := $(shell find $(SRC_DIRS) -type f -name "*.c")
ASM_SOURCES := $(shell find $(SRC_DIRS) -type f -name "*.asm")

C_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst %.asm,$(BUILD_DIR)/%_asm.o,$(ASM_SOURCES))

MULTIBOOT_OBJ := $(BUILD_DIR)/src/multiboot_header.o
FILTERED_ASM_OBJECTS := $(filter-out $(MULTIBOOT_OBJ),$(ASM_OBJECTS))
OBJECTS := $(C_OBJECTS) $(FILTERED_ASM_OBJECTS)

# === Targets ===

.PHONY: all run gdb clean disk-img iso

all: $(KERNEL_BIN) $(KERNEL_ELF) iso

# === Compilation rules ===

# Compile C files
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble ASM files
$(BUILD_DIR)/%_asm.o: %.asm
	@mkdir -p $(dir $@)
	@echo "Assembling $<"
	$(NASM) -f elf32 $< -o $@

# Special rule for multiboot_header.asm
$(MULTIBOOT_OBJ): src/multiboot_header.asm
	@mkdir -p $(dir $@)
	@echo "Assembling multiboot header..."
	$(NASM) -f elf32 $< -o $@

# Link kernel ELF (to ISO_DIR)
$(KERNEL_ELF): $(MULTIBOOT_OBJ) $(OBJECTS) | $(ISO_DIR)
	@echo "Linking kernel ELF..."
	$(LD) $(LDFLAGS) -o $@ $(MULTIBOOT_OBJ) $(OBJECTS)

# Convert ELF to flat binary for GRUB
$(KERNEL_BIN): $(KERNEL_ELF)
	@echo "Generating kernel.bin..."
	$(OBJCOPY) -O binary $< $@

# === Directory creation ===

$(ISO_DIR):
	mkdir -p $(GRUB_DIR)

# === Create ISO ===
iso: $(KERNEL_BIN) $(KERNEL_ELF) $(GRUB_DIR)
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
		-device usb-storage,drive=usbdisk \
		-drive file=usb.img,if=none,id=usbdisk,format=raw \
		-device VGA,vgamem_mb=32 \
		-serial stdio

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

# === Clean ===
clean:
	rm -rf $(BUILD_DIR) $(ISO_FILE) $(KERNEL_ELF) $(KERNEL_BIN)
