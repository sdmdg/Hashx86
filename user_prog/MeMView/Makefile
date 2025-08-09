GPP_PARAMS = -m32 -g -ffreestanding -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-common
ASM_PARAMS = --32 -g
ASM_NASM_PARAMS = -f elf32
objects = Hx86/Hx86.o \
		  Hx86/utils/string.o \
		  Hx86/globals.o \
		  Hx86/debug.o \
		  Hx86/Hgui/Hgui.o \
		  Hx86/Hgui/widget.o \
		  Hx86/Hgui/desktop.o \
          Hx86/Hgui/window.o \
		  Hx86/Hgui/button.o \
		  Hx86/Hgui/label.o \
		  Hx86/memory.o \
		  Hx86/Hsyscalls/syscalls.o \
		  Hx86/Hsyscalls/Hsyscallsgui.o \
		  prog.o

LD_PARAMS = -melf_i386

# Compiling C++ files inside the main directory
%.o: %.cpp
	g++ $(GPP_PARAMS) -o $@ -c $<

# Linking the kernel binary
prog.bin: linker.ld $(objects)
	ld $(LD_PARAMS) -T $< -o $@ $(objects)

# Clean rule: removes object files and the final binary
clean:
	rm -f $(objects) prog.bin

build:
	make clean
	make