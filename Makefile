GPP_PARAMS = -m32 -g -ffreestanding -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions
ASM_PARAMS = --32 -g
ASM_NASM_PARAMS = -f elf32
objects = asm/loader.o \
          asm/interruptstub.o \
		  stdlib.o \
		  debug.o \
          kernel.o \
          console.o \
          core/gdt.o \
          core/ports.o \
		  core/interrupts.o \
		  core/memory.o \
		  core/driver.o \
		  core/drivers/keyboard.o \
		  core/drivers/mouse.o \
		  core/drivers/vbe.o \
		  core/pci.o \
		  gui/fonts/font.o \
		  gui/fonts/segoeui.o \
		  gui/widget.o \
		  gui/desktop.o \
		  gui/window.o \
		  gui/button.o

LD_PARAMS = -melf_i386

# Compiling C++ files inside the main directory
%.o: %.cpp
	g++ $(GPP_PARAMS) -o $@ -c $<

# Compiling C++ files inside the core directory
core/%.o: core/%.cpp
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
	make clean
	make
	qemu-system-i386 -kernel kernel.bin -vga std -serial stdio

run:
	make clean
	make
	make iso
	qemu-system-i386 -cdrom kernel.iso -boot d  -vga std -serial stdio

runvb: kernel.iso
	(killall VirtualBox && sleep 1) || true
	VirtualBox --startvm 'My Operating System' &

iso: kernel.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp kernel.bin iso/boot/kernel.bin
	echo 'set timeout=0'                      > iso/boot/grub/grub.cfg
	echo 'set default=0'                     >> iso/boot/grub/grub.cfg
	echo 'set gfxmode=1024x768x32'         >> iso/boot/grub/grub.cfg
	echo 'set gfxpayload=keep'         >> iso/boot/grub/grub.cfg
	echo 'terminal_output gfxterm'         >> iso/boot/grub/grub.cfg
	echo ''               >> iso/boot/grub/grub.cfg
	echo 'menuentry "My Operating System" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin'      >> iso/boot/grub/grub.cfg
	echo '  boot'      >> iso/boot/grub/grub.cfg
	echo '}'                                 >> iso/boot/grub/grub.cfg
	grub-mkrescue --output=kernel.iso --modules="video gfxterm video_bochs video_cirrus" iso
	rm -rf iso