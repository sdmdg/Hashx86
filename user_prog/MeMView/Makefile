# user_prog/MeMView/Makefile

GPP = g++
LD = ld

# Includes: Look in local include AND libhx86 include
INCLUDES = -Iinclude -I../libhx86/include

# Compiler Flags
CPPFLAGS = -m32 -g -ffreestanding -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-common $(INCLUDES)

# Linker Flags: Link against the static library in ../libhx86
# -L defines where to look, -l defines the library name (libhx86.a -> -lhx86)
LDFLAGS = -melf_i386 -L../libhx86 -lhx86

# Source files for THIS program only
SRCS = prog.cpp
OBJS = prog.o
TARGET = prog.bin

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "[APP] Linking $@"
	@# IMPORTANT: $(LDFLAGS) must come AFTER $(OBJS) for the linker to resolve symbols
	@$(LD) -T linker.ld -o $@ $(OBJS) $(LDFLAGS)

%.o: %.cpp
	@echo "[APP] Compiling $<"
	@$(GPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)


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
