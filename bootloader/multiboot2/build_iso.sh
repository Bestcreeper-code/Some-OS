rm -r iso/boot
mkdir iso/boot
mkdir iso/boot/grub
cp bootloader/multiboot2/grub.cfg iso/boot/grub

grub-mkrescue -o os.iso iso/