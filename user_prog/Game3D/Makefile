# user_prog/Game3D/Makefile

GPP = g++
LD = ld
BUILD_DIR ?= ../../build/user_prog/Game3D
LIBHX86_BUILD_DIR ?= ../../build/user_prog/libhx86

# Includes: Look in local include AND libhx86 include
INCLUDES = -Iinclude -I../libhx86/include

# Compiler Flags
CPPFLAGS = -m32 -O2 -g -ffreestanding -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-common $(INCLUDES)

# Linker Flags: Link against the static library in ../libhx86
LDFLAGS = -melf_i386 -L$(LIBHX86_BUILD_DIR) -lhx86

# Source files for THIS program only
SRCS = prog.cpp Renderer3D.cpp Bitmap.cpp
OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))
TARGET = $(BUILD_DIR)/prog.bin

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "[APP] Linking $@"
	@mkdir -p $(dir $@)
	@$(LD) -T linker.ld -o $@ $(OBJS) $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp
	@echo "[APP] Compiling $<"
	@mkdir -p $(dir $@)
	@$(GPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
