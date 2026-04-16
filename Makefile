#!/usr/bin/make -f

# === Tools ===
CC       = gcc
NASM     = nasm
LD       = ld
OBJCOPY  = objcopy

ARCH    = x86
BOOTLOADER = multiboot2

DEFINES = __ARCH_X86__

# === Directories ===

ISO_DIR = iso
ARCH_DIR = src/arch/$(ARCH)
BOOTLOADERs_DIR = bootloader
CURR_BOOTLOADER_DIR = $(BOOTLOADERs_DIR)/$(BOOTLOADER)

SRC_DIRS  = src FatFs bootloader/$(BOOTLOADER)
BUILD_DIR = build

INCLUDE_DIRS = src FatFs src/config src/headers src/headers/defines \
	src/bootloader                          \
	src/arch/includes src/arch/includes/asm \
	$(ARCH_DIR) $(ARCH_DIR)/elf             \
	$(ARCH_DIR)/memory $(ARCH_DIR)/init     \
	$(ARCH_DIR)/asm $(ARCH_DIR)/cpu         \
	$(ARCH_DIR)/scheduler $(ARCH_DIR)/panic \
	$(ARCH_DIR)/syscalls                    \
	src/drivers src/drivers/ATA src/drivers/PS-2 \
	src/drivers/FS/FAT 						\
	$(BOOTLOADERs_DIR) $(CURR_BOOTLOADER_DIR)

INCLUDES     := $(addprefix -I,$(INCLUDE_DIRS))
DEFINES_FLAGS = $(addprefix -D,$(DEFINES))

CFLAGS  = -m32 -O0 -g -ffreestanding $(INCLUDES) $(DEFINES_FLAGS) \
          -fno-stack-protector -mno-sse -mno-sse2 -fno-tree-vectorize \


LDFLAGS = -m elf_i386 -T src/arch/$(ARCH)/linker.ld -z noexecstack





KERNEL_NOSYMS_ELF = $(BUILD_DIR)/kernel.nosyms.elf
KERNEL_ELF        = $(ISO_DIR)/kernel.elf
KERNEL_BIN        = $(BUILD_DIR)/kernel.bin
DISK_IMG          = disk.img
ISO_FILE          = os.iso

SYMS_BIN = syms.bin
SYMS_OBJ = $(BUILD_DIR)/syms.o


SYMBOLS_SRC        = $(ARCH_DIR)/debug/symbols.c
SYMBOLS_OBJ_NOSYMS = $(BUILD_DIR)/$(ARCH_DIR)/debug/symbols_nosyms.o
SYMBOLS_OBJ_FINAL  = $(BUILD_DIR)/$(ARCH_DIR)/debug/symbols_final.o


C_SOURCES   := $(shell find $(SRC_DIRS) -type f -name "*.c" \
                   | grep -v '$(ARCH_DIR)/debug/symbols\.c')
C_SOURCES += bootloader/bootloader_common.c

ASM_SOURCES := $(shell find $(SRC_DIRS) -type f -name "*.asm")

C_OBJECTS   := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst %.asm,$(BUILD_DIR)/%_asm.o,$(ASM_SOURCES))

BOOT_HEADER_OBJ      := $(BUILD_DIR)/$(CURR_BOOTLOADER_DIR)/boot_header.o
FILTERED_ASM_OBJECTS := $(filter-out $(BOOT_HEADER_OBJ),$(ASM_OBJECTS))


COMMON_OBJECTS := $(C_OBJECTS) $(FILTERED_ASM_OBJECTS)


NOSYMS_OBJECTS := $(COMMON_OBJECTS) $(SYMBOLS_OBJ_NOSYMS)
FINAL_OBJECTS  := $(COMMON_OBJECTS) $(SYMBOLS_OBJ_FINAL)


.PHONY: all run gdb clean disk-img iso

all: $(KERNEL_BIN) $(KERNEL_ELF) iso


$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "  CC  $<"
	$(CC) $(CFLAGS) -c $< -o $@
	@printf "\n\n\n"

$(BUILD_DIR)/%_asm.o: %.asm
	@mkdir -p $(dir $@)
	@echo "  AS  $<"
	$(NASM) -f elf32 $< -o $@
	@printf "\n\n\n"


$(BOOT_HEADER_OBJ): $(CURR_BOOTLOADER_DIR)/boot_header.asm
	@mkdir -p $(dir $@)
	@echo "  AS  [multiboot] $<"
	$(NASM) -f elf32 $< -o $@
	@printf "\n\n\n"


$(SYMBOLS_OBJ_NOSYMS): $(SYMBOLS_SRC)
	@mkdir -p $(dir $@)
	@echo "  CC  [nosyms] $<"
	$(CC) $(CFLAGS) -D__NO_KSYMS -c $< -o $@


$(SYMBOLS_OBJ_FINAL): $(SYMBOLS_SRC) $(SYMS_OBJ)
	@mkdir -p $(dir $@)
	@echo "  CC  [final] $<"
	$(CC) $(CFLAGS) -c $< -o $@


$(KERNEL_NOSYMS_ELF): $(BOOT_HEADER_OBJ) $(NOSYMS_OBJECTS) 
	@echo "  LD  [nosyms] $@"
	$(LD) $(LDFLAGS) -o $@ $(BOOT_HEADER_OBJ) $(NOSYMS_OBJECTS)


$(SYMS_BIN): $(KERNEL_NOSYMS_ELF)
	@echo "  SYM generating $@ from $<"
	sh ./syms_file_maker.sh $< $@


$(SYMS_OBJ): $(SYMS_BIN)
	@mkdir -p $(dir $@)
	@echo "  OC  $@"
	$(OBJCOPY) -I binary -O elf32-i386 -B i386 \
	    --redefine-sym _binary_syms_bin_start=_syms_bin_start \
	    --redefine-sym _binary_syms_bin_end=_syms_bin_end   \
	    --redefine-sym _binary_syms_bin_size=_syms_bin_size \
	    $(SYMS_BIN) $(SYMS_OBJ)


$(KERNEL_ELF): $(SYMS_OBJ) $(SYMBOLS_OBJ_FINAL) $(BOOT_HEADER_OBJ) $(COMMON_OBJECTS) 
	@echo "  LD  [final] $@"
	$(LD) $(LDFLAGS) -o $@ \
	    $(SYMS_OBJ) $(BOOT_HEADER_OBJ) $(SYMBOLS_OBJ_FINAL) $(COMMON_OBJECTS)
	cp $(KERNEL_ELF) ./

# === Generate flat binary ===
$(KERNEL_BIN): $(KERNEL_ELF)
	@echo "  BIN $@"
	$(OBJCOPY) -O binary $< $@


# === ISO ===
iso: $(KERNEL_ELF) $(KERNEL_BIN) $(GRUB_DIR)
	@echo "  ISO $@"
	
	sh $(CURR_BOOTLOADER_DIR)/build_iso.sh

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
		-m 256M \
		-boot d \
		-cdrom $(ISO_FILE) \
		-drive file=$(DISK_IMG),format=raw,if=ide \
		-usb \
		-device VGA,vgamem_mb=32 \
		-display gtk \
		-serial stdio \
		-enable-kvm

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


clean:
	rm -r $(BUILD_DIR) $(ISO_FILE) $(KERNEL_ELF) $(KERNEL_BIN) \
	       $(KERNEL_NOSYMS_ELF) $(SYMS_BIN)