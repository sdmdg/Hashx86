GPP_PARAMS = -m32 -ffreestanding -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions
ASM_PARAMS = --32
objects = asm/loader.o \
          asm/interruptstub.o \
          kernel.o \
          console.o \
          core/gdt.o \
          core/ports.o \
		  core/interrupts.o \
		  core/driver.o \
		  core/drivers/keyboard.o \
		  core/drivers/mouse.o
		  

LD_PARAMS = -melf_i386

# Compiling C++ files inside the main directory
%.o: %.cpp
	g++ $(GPP_PARAMS) -o $@ -c $<

# Compiling C++ files inside the core directory
core/%.o: core/%.cpp
	g++ $(GPP_PARAMS) -o $@ -c $<

core/drivers%.o: core/drivers%.cpp
	g++ $(GPP_PARAMS) -o $@ -c $<

# Compiling assembly files
asm/%.o: asm/%.s
	as $(ASM_PARAMS) -o $@ $<

# Linking the kernel binary
kernel.bin: linker.ld $(objects)
	ld $(LD_PARAMS) -T $< -o $@ $(objects)

# Install the kernel binary
install: kernel.bin
	sudo cp kernel.bin /boot/kernel.bin

# Clean rule: removes object files and the final binary
clean:
	rm -f $(objects) kernel.bin

run:
	qemu-system-i386 -kernel kernel.bin -d cpu_reset -D ./cpu_reset.log

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
	echo ''                                  >> iso/boot/grub/grub.cfg
	echo 'menuentry "My Operating System" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin'    >> iso/boot/grub/grub.cfg
	echo '  boot'                            >> iso/boot/grub/grub.cfg
	echo '}'                                 >> iso/boot/grub/grub.cfg
	grub-mkrescue --output=kernel.iso iso
	rm -rf iso