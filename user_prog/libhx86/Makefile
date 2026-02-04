# user_prog/shared/Makefile

# Compiler Settings
GPP = g++
AR = ar

# Flags: Note the include path points to the local 'include' folder
CPPFLAGS = -m32 -g -ffreestanding -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-common -Iinclude

# Automatically find all .cpp files inside Hx86/ and subdirectories
SRCS := $(shell find Hx86 -name '*.cpp')
OBJS := $(SRCS:.cpp=.o)

# The Output Library
TARGET = libhx86.a

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "[LIB] Archiving $@"
	@$(AR) rcs $@ $(OBJS)

%.o: %.cpp
	@echo "[LIB] Compiling $<"
	@$(GPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)