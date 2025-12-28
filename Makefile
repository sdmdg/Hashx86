GPP_PARAMS = -m32 -g -ffreestanding -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-common
ASM_PARAMS = --32 -g
ASM_NASM_PARAMS = -f elf32
objects = asm/loader.o \
          asm/common_handler.o \
          asm/load_gdt.o \
		  asm/load_tss.o \
		  utils/string.o \
		  core/globals.o \
		  core/pmm.o \
		  core/paging.o \
		  stdlib.o \
		  debug.o \
          kernel.o \
		  core/gdt.o \
          core/ports.o \
		  core/interrupts.o \
		  core/syscalls.o \
		  gui/Hgui.o \
		  core/memory.o \
		  core/driver.o \
		  core/drivers/GraphicsDriver.o \
		  core/drivers/ModuleLoader.o \
		  core/drivers/SymbolTable.o \
		  core/drivers/keyboard.o \
		  core/drivers/mouse.o \
		  core/drivers/vbe.o \
		  core/drivers/ata.o \
		  core/process.o \
		  core/thread.o \
		  core/elf.o \
		  core/pci.o \
		  core/filesystem/msdospart.o \
		  core/filesystem/FAT32.o \
		  core/filesystem/File.o \
		  gui/renderer/nina.o \
		  gui/fonts/font.o \
		  gui/bmp.o \
		  gui/widget.o \
		  gui/desktop.o \
		  gui/window.o \
		  gui/button.o \
		  gui/inputbox.o \
		  gui/elements/window_action_button.o \
		  gui/elements/window_action_button_round.o \
		  gui/label.o
#		  gui/messagebox.o 
LD_PARAMS = -melf_i386

# Compiling C++ files inside the main directory
%.o: %.cpp
	g++ $(GPP_PARAMS) -o $@ -c $<

# Compiling C++ files inside the core directory
core/%.o: core/%.cpp
	g++ $(GPP_PARAMS) -o $@ -c $<

core/%.o: %.c
	g++ $(GPP_PARAMS) -o $@ -c $<

# Compiling C++ files inside the gui directory
gui/%.o: gui/%.cpp
	g++ $(GPP_PARAMS) -o $@ -c $<

# Compiling assembly files
asm/%.o: asm/%.s
	as $(ASM_PARAMS) -o $@ $<

# Compiling NASM assembly files
asm/%.o: asm/%.asm
	nasm $(ASM_NASM_PARAMS) -o $@ $<

# Linking the kernel binary
kernel.bin: linker.ld $(objects)
	ld $(LD_PARAMS) -T $< -o $@ $(objects)

# Install the kernel binary
install: kernel.bin
	sudo cp kernel.bin /boot/kernel.bin

# Clean rule: removes object files and the final binary
clean:
	rm -f $(objects) kernel.bin

runq:
	qemu-system-i386 -cdrom kernel.iso -boot d  -vga std -serial stdio -m 2G -hda HDD.vdi -d int,cpu_reset -D ./log.txt

run:
	make clean
	make
	make iso
	qemu-system-i386 -cdrom kernel.iso -boot d  -vga std -serial stdio -m 2G -hda HDD.vdi -d int,cpu_reset -D ./log.txt
# -bios /usr/share/ovmf/OVMF.fd

build:
	make
	make iso
	qemu-system-i386 -cdrom kernel.iso -boot d  -vga std -serial stdio -m 2G -hda HDD.vdi -d int,cpu_reset -D ./log.txt

hdd:
# 	Cleanup from previous mounts
	-sudo umount /mnt/vdi_p1
	-sudo qemu-nbd --disconnect /dev/nbd0
# 	1. Load the NBD module
	sudo modprobe nbd max_part=16
# 	2. Connect VHD to /dev/nbd0
	sudo qemu-nbd --connect=/dev/nbd0 HDD.vdi
# 	3. Mount Partition 1 (p1)
	sudo mkdir -p /mnt/vdi_p1
	sudo mount /dev/nbd0p1 /mnt/vdi_p1
# 	4. Copy Files
	-sudo mkdir -p /mnt/vdi_p1/bin
	-sudo mkdir -p /mnt/vdi_p1/fonts
	-sudo mkdir -p /mnt/vdi_p1/bitmaps
	-sudo mkdir -p /mnt/vdi_p1/drivers

	sudo cp bin/bitmaps/boot.bmp /mnt/vdi_p1/bitmaps/boot.bmp
	sudo cp bin/bitmaps/icon.bmp /mnt/vdi_p1/bitmaps/icon.bmp
	sudo cp bin/bitmaps/cursor.bmp /mnt/vdi_p1/bitmaps/cursor.bmp
	sudo cp bin/bitmaps/desktop.bmp /mnt/vdi_p1/bitmaps/desktop.bmp
	sudo cp bin/bitmaps/panic.bmp /mnt/vdi_p1/bitmaps/panic.bmp

#	sudo rm -f /mnt/vdi_p1/drivers/bga.o
	sudo cp drivers/bga.sys /mnt/vdi_p1/drivers/bga.sys

	sudo cp bin/fonts/segoeui.bin /mnt/vdi_p1/fonts/segoeui.bin

	sudo cp user_prog/MeMView/prog.bin /mnt/vdi_p1/bin/MeMView.bin
	sudo cp user_prog/test/prog.bin /mnt/vdi_p1/bin/test.bin
# 	5. Cleanup
	sudo umount /mnt/vdi_p1
	sudo qemu-nbd --disconnect /dev/nbd0

runvb: kernel.iso
	(killall VirtualBox && sleep 1) || true
	VirtualBox --startvm 'My Operating System' &

iso: kernel.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	mkdir iso/boot/fonts
	cp kernel.bin iso/boot/kernel.bin
#	cp bin/fonts/segoeui.bin iso/boot/fonts/segoeui.bin
	echo 'set timeout=0'                      				>> iso/boot/grub/grub.cfg
	echo 'set default=0'                     				>> iso/boot/grub/grub.cfg
#	echo 'set gfxmode=1152x864x32'         					>> iso/boot/grub/grub.cfg
#	echo 'set gfxpayload=keep'         						>> iso/boot/grub/grub.cfg
	echo 'terminal_output gfxterm'         					>> iso/boot/grub/grub.cfg
	echo ''               >> iso/boot/grub/grub.cfg
	echo 'menuentry "My Operating System" {' 				>> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin'      				>> iso/boot/grub/grub.cfg
#	echo '  module /boot/fonts/segoeui.bin'      			>> iso/boot/grub/grub.cfg
	echo '  boot'      										>> iso/boot/grub/grub.cfg
	echo '}'                                 				>> iso/boot/grub/grub.cfg
	grub-mkrescue --output=kernel.iso --modules="video gfxterm video_bochs video_cirrus" iso
	rm -rf iso

prog:
	make hdd
	make runq