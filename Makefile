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
		  core/drivers/keyboard.o \
		  core/drivers/mouse.o \
		  core/drivers/vbe.o \
		  core/drivers/ata.o \
		  core/process.o \
		  core/thread.o \
		  core/elf.o \
		  core/pci.o \
		  gui/renderer/nina.o \
		  gui/fonts/font.o \
		  gui/fonts/segoeui.o \
		  gui/widget.o \
		  gui/desktop.o \
		  gui/window.o \
		  gui/button.o \
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

runvb: kernel.iso
	(killall VirtualBox && sleep 1) || true
	VirtualBox --startvm 'My Operating System' &

iso: kernel.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	mkdir iso/boot/font
	cp kernel.bin iso/boot/kernel.bin
	cp fonts/8.png iso/boot/font/8.png
	cp user_prog/test/prog.bin iso/boot/prog.bin
#	cp user_prog/test/prog1.bin iso/boot/prog1.bin
	echo 'set timeout=0'                      > iso/boot/grub/grub.cfg
	echo 'set default=0'                     >> iso/boot/grub/grub.cfg
#	echo 'set gfxmode=1152x864x32'         >> iso/boot/grub/grub.cfg
#	echo 'set gfxpayload=keep'         >> iso/boot/grub/grub.cfg
	echo 'terminal_output gfxterm'         >> iso/boot/grub/grub.cfg
	echo ''               >> iso/boot/grub/grub.cfg
	echo 'menuentry "My Operating System" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin'      >> iso/boot/grub/grub.cfg
	echo '  module /boot/font/8.png'      >> iso/boot/grub/grub.cfg
	echo '  module /boot/prog.bin'      >> iso/boot/grub/grub.cfg
#	echo '  module /boot/prog1.bin'      >> iso/boot/grub/grub.cfg
	echo '  boot'      >> iso/boot/grub/grub.cfg
	echo '}'                                 >> iso/boot/grub/grub.cfg
	grub-mkrescue --output=kernel.iso --modules="video gfxterm video_bochs video_cirrus" iso
	rm -rf iso

prog:
	make iso
	make runq