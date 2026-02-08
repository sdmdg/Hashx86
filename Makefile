GPP_PARAMS = -m32 -g -ffreestanding -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-common -fno-omit-frame-pointer
ASM_PARAMS = --32 -g
ASM_NASM_PARAMS = -f elf32
objects = asm/common_handler.o \
          asm/load_gdt.o \
          asm/load_tss.o \
          asm/loader.o \
          audio/wav.o \
          core/driver.o \
          core/drivers/ata.o \
          core/drivers/AudioMixer.o \
          core/drivers/GraphicsDriver.o \
          core/drivers/keyboard.o \
          core/drivers/ModuleLoader.o \
          core/drivers/mouse.o \
          core/drivers/SymbolTable.o \
          core/drivers/vbe.o \
          core/elf.o \
          core/filesystem/FAT32.o \
          core/filesystem/File.o \
          core/filesystem/msdospart.o \
          core/gdt.o \
          core/globals.o \
          core/interrupts.o \
          core/KernelSymbolResolver.o \
          core/memory.o \
          core/paging.o \
          core/pci.o \
          core/pmm.o \
          core/ports.o \
          core/scheduler.o \
          core/syscalls.o \
          debug.o \
          gui/bmp.o \
          gui/button.o \
          gui/desktop.o \
          gui/elements/window_action_button.o \
          gui/elements/window_action_button_round.o \
          gui/fonts/font.o \
          gui/Hgui.o \
          gui/label.o \
          gui/renderer/nina.o \
          gui/widget.o \
          gui/window.o \
          kernel.o \
          stdlib.o \
          stdlib/math.o \
          utils/string.o

LD_PARAMS = -melf_i386 -Map kernel.map

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

# Compiling C++ files inside the stdlib directory
stdlib/%.o: stdlib/%.cpp
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

build:
	make clean
	make
	make -C user_prog clean
	make -C user_prog
	make iso
	make hdd
	make runq

runq:
	qemu-system-i386 -cdrom kernel.iso -boot d -vga std -serial stdio -m 1G \
    -drive file=HDD.vdi,format=vdi \
    -audiodev pa,id=snd0 \
    -device ac97,audiodev=snd0

run:
	make clean
	make
	make iso
	make runq

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
	-sudo mkdir -p /mnt/vdi_p1/audio

	-sudo cp kernel.map /mnt/vdi_p1/kernel.map

#	-sudo cp bin/obj.obj /mnt/vdi_p1/obj.obj
#	-sudo cp bin/obj1.obj /mnt/vdi_p1/obj1.obj
#	-sudo cp bin/map.bmp /mnt/vdi_p1/map.bmp
#	-sudo cp bin/sky.bmp /mnt/vdi_p1/sky.bmp
	-sudo cp bin/audio/boot.wav /mnt/vdi_p1/audio/boot.wav
	-sudo cp bin/bitmaps/boot.bmp /mnt/vdi_p1/bitmaps/boot.bmp
	-sudo cp bin/bitmaps/icon.bmp /mnt/vdi_p1/bitmaps/icon.bmp
	-sudo cp bin/bitmaps/cursor.bmp /mnt/vdi_p1/bitmaps/cursor.bmp
	-sudo cp bin/bitmaps/desktop.bmp /mnt/vdi_p1/bitmaps/desktop.bmp
	-sudo cp bin/bitmaps/panic.bmp /mnt/vdi_p1/bitmaps/panic.bmp

#	-sudo cp bin/sound.wav /mnt/vdi_p1/sound.wav
#	-sudo rm -f /mnt/vdi_p1/drivers/ac97.sys

	-sudo cp drivers/bga.sys /mnt/vdi_p1/drivers/bga.sys
	-sudo cp drivers/ac97.sys /mnt/vdi_p1/drivers/ac97.sys

	-sudo cp bin/fonts/segoeui.bin /mnt/vdi_p1/fonts/segoeui.bin

	-sudo cp user_prog/MeMView/prog.bin /mnt/vdi_p1/bin/MeMView.bin
	-sudo cp user_prog/test/prog.bin /mnt/vdi_p1/bin/test.bin
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


# -----------------------------------
# CODE QUALITY TOOLS
# Check style (Report Only)
check-style:
	@echo "--- Checking Code Formatting ---"
	@find . -name "*.cpp" -o -name "*.c" -o -name "*.h" | xargs clang-format --dry-run -Werror
	@echo "Style check passed."

# Check Logic (Report Only)
check-bugs:
	@echo "--- Static Analysis ---"
	@cppcheck --enable=warning,performance,portability \
		--suppress=missingIncludeSystem \
		--inline-suppr \
		--quiet \
		.

# Check Headers (Report Only)
check-headers:
	@./check_headers.sh

# Check EOF (Report Only)
check-eof:
	@./check_eof.sh

# Master Check
check: check-style check-bugs check-eof check-headers

# Auto-Fix-Style
fix-style:
	@echo "--- Applying Auto-Formatting ---"
	@find . -name "*.cpp" -o -name "*.c" -o -name "*.h" | xargs clang-format -i -style=file
	@echo "Code formatted."
