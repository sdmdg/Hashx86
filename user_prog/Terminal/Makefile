# user_prog/Terminal/Makefile

GPP = g++
LD = ld
BUILD_DIR ?= ../../build/user_prog/Terminal
LIBHX86_BUILD_DIR ?= ../../build/user_prog/libhx86

INCLUDES = -Iinclude -I../libhx86/include

CPPFLAGS = -m32 -O2 -g -ffreestanding -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-common $(INCLUDES)

LDFLAGS = -melf_i386 -L$(LIBHX86_BUILD_DIR) -lhx86

SRCS = prog.cpp
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
