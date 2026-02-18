# user_prog/Game3D/Makefile

GPP = g++
LD = ld

# Includes: Look in local include AND libhx86 include
INCLUDES = -Iinclude -I../libhx86/include

# Compiler Flags
CPPFLAGS = -m32 -g -ffreestanding -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-common $(INCLUDES)

# Linker Flags: Link against the static library in ../libhx86
LDFLAGS = -melf_i386 -L../libhx86 -lhx86

# Source files for THIS program only
SRCS = prog.cpp Renderer3D.cpp Bitmap.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = prog.bin

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "[APP] Linking $@"
	@$(LD) -T linker.ld -o $@ $(OBJS) $(LDFLAGS)

%.o: %.cpp
	@echo "[APP] Compiling $<"
	@$(GPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
