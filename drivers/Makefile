GXX = g++
GPP_PARAMS = -m32 -g -ffreestanding -I../include -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-common -c -fno-pic

# List of drivers to build
DRIVERS = bga.sys \
		  ac97.sys

all: $(DRIVERS)

# Compiling C++ files inside the drivers directory
%.sys: %.cpp
	$(GXX) $(GPP_PARAMS) -o $@ $<

clean:
	rm -f *.sys