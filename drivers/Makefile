GXX = g++
GPP_PARAMS = -m32 -g -ffreestanding -I../include -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-common -c -fno-pic
BUILD_DIR ?= ../build/drivers

# List of drivers to build
DRIVERS = bga.sys \
		  ac97.sys
DRIVER_TARGETS = $(addprefix $(BUILD_DIR)/,$(DRIVERS))

all: $(DRIVER_TARGETS)

# Compiling C++ files inside the drivers directory
$(BUILD_DIR)/%.sys: %.cpp
	@mkdir -p $(dir $@)
	$(GXX) $(GPP_PARAMS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)