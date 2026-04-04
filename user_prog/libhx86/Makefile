# user_prog/shared/Makefile

# Compiler Settings
GPP = g++
AR = ar
BUILD_DIR ?= ../../build/user_prog/libhx86

INCLUDES = -Iinclude

# Flags: Note the include path points to the local 'include' folder
CPPFLAGS = -m32 -g -ffreestanding -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-common $(INCLUDES)

# Automatically find all .cpp files inside Hx86/ and subdirectories
SRCS := $(shell find Hx86 -name '*.cpp')
OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# The Output Library
TARGET = $(BUILD_DIR)/libhx86.a

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "[LIB] Archiving $@"
	@mkdir -p $(dir $@)
	@$(AR) rcs $@ $(OBJS)

$(BUILD_DIR)/%.o: %.cpp
	@echo "[LIB] Compiling $<"
	@mkdir -p $(dir $@)
	@$(GPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)