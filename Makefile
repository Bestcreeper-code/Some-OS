#!/usr/bin/make -f

# === Tools ===
CC       = gcc
NASM     = nasm
LD       = ld
OBJCOPY  = objcopy

ARCH    = x86
DEFINES = __ARCH_X86__

ARCH_DIR = src/arch/$(ARCH)

INCLUDE_DIRS = src FatFs src/config src/headers src/headers/defines \
	src/bootloader                          \
	src/arch/includes src/arch/includes/asm \
	$(ARCH_DIR) $(ARCH_DIR)/elf             \
	$(ARCH_DIR)/memory $(ARCH_DIR)/init     \
	$(ARCH_DIR)/asm $(ARCH_DIR)/cpu         \
	$(ARCH_DIR)/scheduler $(ARCH_DIR)/panic \
	$(ARCH_DIR)/syscalls                    \
	src/drivers src/drivers/ATA src/drivers/PS-2 \
	src/drivers/FS/FAT

INCLUDES     := $(addprefix -I,$(INCLUDE_DIRS))
DEFINES_FLAGS = $(addprefix -D,$(DEFINES))

CFLAGS  = -m32 -O0 -g -ffreestanding $(INCLUDES) $(DEFINES_FLAGS) \
          -fno-stack-protector -mno-sse -mno-sse2 -fno-tree-vectorize \


LDFLAGS = -m elf_i386 -T src/arch/$(ARCH)/linker.ld -z noexecstack

# === Directories ===
SRC_DIRS  = src FatFs
BUILD_DIR = build
ISO_DIR   = iso/boot
GRUB_DIR  = $(ISO_DIR)/grub

# === Output files ===
KERNEL_NOSYMS_ELF = kernel.nosyms.elf
KERNEL_ELF        = $(ISO_DIR)/kernel.elf
KERNEL_BIN        = kernel.bin
DISK_IMG          = disk.img
ISO_FILE          = os.iso

SYMS_BIN = syms.bin
SYMS_OBJ = $(BUILD_DIR)/syms.o


SYMBOLS_SRC        = src/debug/symbols.c
SYMBOLS_OBJ_NOSYMS = $(BUILD_DIR)/src/debug/symbols_nosyms.o
SYMBOLS_OBJ_FINAL  = $(BUILD_DIR)/src/debug/symbols_final.o

# === Source discovery (exclude symbols.c — handled separately) ===
C_SOURCES   := $(shell find $(SRC_DIRS) -type f -name "*.c" \
                   | grep -v 'src/debug/symbols\.c')
ASM_SOURCES := $(shell find $(SRC_DIRS) -type f -name "*.asm")

C_OBJECTS   := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst %.asm,$(BUILD_DIR)/%_asm.o,$(ASM_SOURCES))

MULTIBOOT_OBJ        := $(BUILD_DIR)/src/multiboot_header.o
FILTERED_ASM_OBJECTS := $(filter-out $(MULTIBOOT_OBJ),$(ASM_OBJECTS))

# Common objects (no symbols.c, no multiboot)
COMMON_OBJECTS := $(C_OBJECTS) $(FILTERED_ASM_OBJECTS)

# Objects for each link pass
NOSYMS_OBJECTS := $(COMMON_OBJECTS) $(SYMBOLS_OBJ_NOSYMS)
FINAL_OBJECTS  := $(COMMON_OBJECTS) $(SYMBOLS_OBJ_FINAL)

# === Targets ===
.PHONY: all run gdb clean disk-img iso

all: $(KERNEL_BIN) $(KERNEL_ELF) iso

# === Generic compilation rules ===
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "  CC  $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%_asm.o: %.asm
	@mkdir -p $(dir $@)
	@echo "  AS  $<"
	$(NASM) -f elf32 $< -o $@

# === Special: multiboot header ===
$(MULTIBOOT_OBJ): src/bootloader/multiboot1/multiboot_header.asm
	@mkdir -p $(dir $@)
	@echo "  AS  [multiboot] $<"
	$(NASM) -f elf32 $< -o $@

# === Pass-1 symbols.o  (placeholder, __NO_KSYMS) ===
$(SYMBOLS_OBJ_NOSYMS): $(SYMBOLS_SRC)
	@mkdir -p $(dir $@)
	@echo "  CC  [nosyms] $<"
	$(CC) $(CFLAGS) -D__NO_KSYMS -c $< -o $@

# === Pass-2 symbols.o  (real data, depends on syms.o being present) ===
$(SYMBOLS_OBJ_FINAL): $(SYMBOLS_SRC) $(SYMS_OBJ)
	@mkdir -p $(dir $@)
	@echo "  CC  [final] $<"
	$(CC) $(CFLAGS) -c $< -o $@

# === Step 1: link nosyms kernel ===
$(KERNEL_NOSYMS_ELF): $(MULTIBOOT_OBJ) $(NOSYMS_OBJECTS) | $(ISO_DIR)
	@echo "  LD  [nosyms] $@"
	$(LD) $(LDFLAGS) -o $@ $(MULTIBOOT_OBJ) $(NOSYMS_OBJECTS)

# === Step 2: extract symbol table → syms.bin ===
$(SYMS_BIN): $(KERNEL_NOSYMS_ELF)
	@echo "  SYM generating $@ from $<"
	sh ./syms_file_maker.sh $< $@

# === Step 3: wrap syms.bin into an ELF object ===
$(SYMS_OBJ): $(SYMS_BIN)
	@mkdir -p $(dir $@)
	@echo "  OC  $@"
	$(OBJCOPY) -I binary -O elf32-i386 -B i386 \
	    --redefine-sym _binary_syms_bin_start=_syms_bin_start \
	    --redefine-sym _binary_syms_bin_end=_syms_bin_end   \
	    --redefine-sym _binary_syms_bin_size=_syms_bin_size \
	    $(SYMS_BIN) $(SYMS_OBJ)

# === Step 4: link final kernel (syms.o + re-compiled symbols_final.o) ===
#
# Link order matters: syms.o must come BEFORE symbols_final.o so the
# _syms_bin_start/_syms_bin_end symbols are defined before they are used.
#
$(KERNEL_ELF): $(SYMS_OBJ) $(SYMBOLS_OBJ_FINAL) $(MULTIBOOT_OBJ) $(COMMON_OBJECTS) | $(ISO_DIR)
	@echo "  LD  [final] $@"
	$(LD) $(LDFLAGS) -o $@ \
	    $(SYMS_OBJ) $(MULTIBOOT_OBJ) $(SYMBOLS_OBJ_FINAL) $(COMMON_OBJECTS)
	cp $(KERNEL_ELF) ./

# === Generate flat binary ===
$(KERNEL_BIN): $(KERNEL_ELF)
	@echo "  BIN $@"
	$(OBJCOPY) -O binary $< $@

# === Directories ===
$(ISO_DIR):
	mkdir -p $(GRUB_DIR)

# === ISO ===
iso: $(KERNEL_ELF) $(KERNEL_BIN) $(GRUB_DIR)
	@echo "  ISO $@"
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

# === Clean ===
clean:
	rm -r $(BUILD_DIR) $(ISO_FILE) $(KERNEL_ELF) $(KERNEL_BIN) \
	       $(KERNEL_NOSYMS_ELF) $(SYMS_BIN)