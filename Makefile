KDBG_ENABLE ?= 1
KDBG_LEVEL ?= 1

GPP_PARAMS = -m32 -g -ffreestanding -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-common -fno-omit-frame-pointer -DKDBG_ENABLE=$(KDBG_ENABLE) -DKDBG_LEVEL=$(KDBG_LEVEL)
ASM_PARAMS = --32 -g
ASM_NASM_PARAMS = -f elf32
BUILD_DIR = build
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
KERNEL_MAP = $(BUILD_DIR)/kernel.map
KERNEL_ISO = $(BUILD_DIR)/kernel.iso
ISO_DIR = $(BUILD_DIR)/iso
objects = asm/common_handler.o \
          asm/load_gdt.o \
          asm/load_tss.o \
          asm/loader.o \
          audio/wav.o \
          core/driver.o \
          core/drivers/ata.o \
          core/drivers/AudioMixer.o \
          core/drivers/GraphicsDriver.o \
          core/drivers/keyboard.o \
          core/drivers/ModuleLoader.o \
          core/drivers/mouse.o \
          core/drivers/SymbolTable.o \
          core/drivers/vbe.o \
          core/elf.o \
          core/filesystem/FAT32.o \
          core/filesystem/File.o \
          core/filesystem/msdospart.o \
		  core/CrashReporter.o \
          core/gdt.o \
          core/globals.o \
          core/interrupts.o \
          core/KernelSymbolResolver.o \
          core/memory.o \
          core/paging.o \
          core/pci.o \
          core/pmm.o \
          core/ports.o \
          core/scheduler.o \
          core/syscalls.o \
          debug.o \
          gui/bmp.o \
          gui/button.o \
          gui/desktop.o \
          gui/elements/window_action_button.o \
          gui/elements/window_action_button_round.o \
          gui/fonts/font.o \
          gui/Hgui.o \
		  gui/infodialog.o \
          gui/label.o \
          gui/listview.o \
          gui/progressbar.o \
          gui/renderer/nina.o \
		  gui/terminalview.o \
          gui/taskbar.o \
          gui/widget.o \
          gui/window.o \
          kernel.o \
          stdlib.o \
          stdlib/math.o \
          utils/string.o

build_objects = $(addprefix $(BUILD_DIR)/,$(objects))

LD_PARAMS = -melf_i386 -Map $(KERNEL_MAP)
QEMU_DISK = HDD.vdi
RUNQ_DELAY ?= 1

.PHONY: all clean build run runq hdd runvb iso prog install drivers user-programs

all: $(KERNEL_BIN)

# Compiling C++ files inside the main directory
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	g++ $(GPP_PARAMS) -o $@ -c $<

# Compiling C++ files inside the core directory
$(BUILD_DIR)/core/%.o: %.c
	@mkdir -p $(dir $@)
	g++ $(GPP_PARAMS) -o $@ -c $<

# Compiling C++ files inside the gui directory
# Compiling assembly files
$(BUILD_DIR)/asm/%.o: asm/%.s
	@mkdir -p $(dir $@)
	as $(ASM_PARAMS) -o $@ $<

# Compiling NASM assembly files
$(BUILD_DIR)/asm/%.o: asm/%.asm
	@mkdir -p $(dir $@)
	nasm $(ASM_NASM_PARAMS) -o $@ $<

# Linking the kernel binary
$(KERNEL_BIN): linker.ld $(build_objects)
	@mkdir -p $(dir $@)
	ld $(LD_PARAMS) -T $< -o $@ $(build_objects)

# Install the kernel binary
install: $(KERNEL_BIN)
	sudo cp $(KERNEL_BIN) /boot/kernel.bin

# Clean rule: removes object files and the final binary
clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C drivers clean
	$(MAKE) -C user_prog clean

drivers:
	$(MAKE) -C drivers

user-programs:
	$(MAKE) -C user_prog

build:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) drivers
	$(MAKE) user-programs
	$(MAKE) iso
	$(MAKE) hdd
	@echo "[BUILD] Waiting $(RUNQ_DELAY)s to release the HDD file..."
	@sleep $(RUNQ_DELAY)
	$(MAKE) runq

runq: $(KERNEL_ISO)
	qemu-system-i386 -cdrom $(KERNEL_ISO) -boot d -vga std -serial stdio -m 1G \
    -drive file=$(QEMU_DISK),format=vdi \
    -audiodev pa,id=snd0 \
    -device ac97,audiodev=snd0

run:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) iso
	$(MAKE) runq

hdd: $(KERNEL_BIN) drivers user-programs
# 	Cleanup from previous mounts
	-sudo umount /mnt/vdi_p1
	-sudo qemu-nbd --disconnect /dev/nbd0
# 	1. Load the NBD module
	-sudo modprobe nbd max_part=16
# 	2. Connect VHD to /dev/nbd0
	-sudo qemu-nbd --connect=/dev/nbd0 HDD.vdi
# 	3. Mount Partition 1 (p1)
	-sudo mkdir -p /mnt/vdi_p1
	-sudo mount /dev/nbd0p1 /mnt/vdi_p1

# 	4. Copy Files
	-sudo mkdir -p /mnt/vdi_p1/bin
	-sudo mkdir -p /mnt/vdi_p1/fonts
	-sudo mkdir -p /mnt/vdi_p1/bitmaps
	-sudo mkdir -p /mnt/vdi_p1/drivers
	-sudo mkdir -p /mnt/vdi_p1/audio
	-sudo mkdir -p /mnt/vdi_p1/SYS32
	-sudo mkdir -p /mnt/vdi_p1/ProgFile/Game3D

	-sudo cp $(KERNEL_MAP) /mnt/vdi_p1/kernel.map

	-sudo cp bin/audio/boot.wav /mnt/vdi_p1/audio/boot.wav
	-sudo cp bin/bitmaps/boot.bmp /mnt/vdi_p1/bitmaps/boot.bmp
	-sudo cp bin/bitmaps/icon.bmp /mnt/vdi_p1/bitmaps/icon.bmp
	-sudo cp bin/bitmaps/cursor.bmp /mnt/vdi_p1/bitmaps/cursor.bmp
	-sudo cp bin/bitmaps/desktop.bmp /mnt/vdi_p1/bitmaps/desktop.bmp
	-sudo cp bin/bitmaps/panic.bmp /mnt/vdi_p1/bitmaps/panic.bmp

	-sudo cp bin/ProgFile/Game3D/obj.obj /mnt/vdi_p1/ProgFile/Game3D/obj.obj
	-sudo cp bin/ProgFile/Game3D/floor.obj /mnt/vdi_p1/ProgFile/Game3D/floor.obj
	-sudo cp bin/ProgFile/Game3D/map.bmp /mnt/vdi_p1/ProgFile/Game3D/map.bmp
	-sudo cp bin/ProgFile/Game3D/sky.bmp /mnt/vdi_p1/ProgFile/Game3D/sky.bmp

#	-sudo cp bin/sound.wav /mnt/vdi_p1/sound.wav
#	-sudo rm -f /mnt/vdi_p1/drivers/ac97.sys

	-sudo cp $(BUILD_DIR)/drivers/bga.sys /mnt/vdi_p1/drivers/bga.sys
	-sudo cp $(BUILD_DIR)/drivers/ac97.sys /mnt/vdi_p1/drivers/ac97.sys

	-sudo cp bin/fonts/segoeui.bin /mnt/vdi_p1/fonts/segoeui.bin

	-sudo cp $(BUILD_DIR)/user_prog/MeMView/prog.bin /mnt/vdi_p1/SYS32/MeMView.bin
	-sudo cp $(BUILD_DIR)/user_prog/test/prog.bin /mnt/vdi_p1/SYS32/test.bin
	-sudo cp $(BUILD_DIR)/user_prog/Explorer/prog.bin /mnt/vdi_p1/SYS32/Explorer.bin
	-sudo cp $(BUILD_DIR)/user_prog/Terminal/prog.bin /mnt/vdi_p1/SYS32/TERMINAL.BIN
	-sudo cp $(BUILD_DIR)/user_prog/Game3D/prog.bin /mnt/vdi_p1/ProgFile/Game3D/Game3D.bin

# 	5. Cleanup
	sudo umount /mnt/vdi_p1
	sudo qemu-nbd --disconnect /dev/nbd0

runvb: $(KERNEL_ISO)
	(killall VirtualBox && sleep 1) || true
	VirtualBox --startvm 'My Operating System' &

iso: $(KERNEL_ISO)

$(KERNEL_ISO): $(KERNEL_BIN)
	rm -rf $(ISO_DIR)
	mkdir -p $(ISO_DIR)/boot/grub
	mkdir -p $(ISO_DIR)/boot/fonts
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/kernel.bin
#	cp bin/fonts/segoeui.bin iso/boot/fonts/segoeui.bin
	echo 'set timeout=0'                      				>> $(ISO_DIR)/boot/grub/grub.cfg
	echo 'set default=0'                     				>> $(ISO_DIR)/boot/grub/grub.cfg
#	echo 'set gfxmode=1152x864x32'         					>> iso/boot/grub/grub.cfg
#	echo 'set gfxpayload=keep'         						>> iso/boot/grub/grub.cfg
	echo 'terminal_output gfxterm'         					>> $(ISO_DIR)/boot/grub/grub.cfg
	echo ''               >> $(ISO_DIR)/boot/grub/grub.cfg
	echo 'menuentry "My Operating System" {' 				>> $(ISO_DIR)/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin'      				>> $(ISO_DIR)/boot/grub/grub.cfg
#	echo '  module /boot/fonts/segoeui.bin'      			>> iso/boot/grub/grub.cfg
	echo '  boot'      										>> $(ISO_DIR)/boot/grub/grub.cfg
	echo '}'                                 				>> $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue --output=$(KERNEL_ISO) --modules="video gfxterm video_bochs video_cirrus" $(ISO_DIR)
	rm -rf $(ISO_DIR)

prog:
	$(MAKE) hdd
	@echo "[PROG] Waiting $(RUNQ_DELAY)s before runq..."
	@sleep $(RUNQ_DELAY)
	$(MAKE) runq


# -----------------------------------
# CODE QUALITY TOOLS
# Check style (Report Only)
check-style:
	@echo "--- Checking Code Formatting ---"
	@find . -name "*.cpp" -o -name "*.c" -o -name "*.h" | xargs clang-format --dry-run -Werror
	@echo "Style check passed."

# Check Logic (Report Only)
check-bugs:
	@echo "--- Static Analysis ---"
	@cppcheck --enable=warning,performance,portability \
		--suppress=missingIncludeSystem \
		--inline-suppr \
		--quiet \
		.

# Check Headers (Report Only)
check-headers:
	@./check_headers.sh

# Check EOF (Report Only)
check-eof:
	@./check_eof.sh

# Master Check
check: check-style check-bugs check-eof check-headers

# Auto-Fix-Style
fix-style:
	@echo "--- Applying Auto-Formatting ---"
	@find . -name "*.cpp" -o -name "*.c" -o -name "*.h" | xargs clang-format -i -style=file
	@echo "Code formatted."
