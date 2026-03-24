# user_prog/Terminal/Makefile

GPP = g++
LD = ld

INCLUDES = -Iinclude -I../libhx86/include

CPPFLAGS = -m32 -O2 -g -ffreestanding -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-common $(INCLUDES)

LDFLAGS = -melf_i386 -L../libhx86 -lhx86

SRCS = prog.cpp
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
