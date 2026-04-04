SUBDIRS = MeMView\
		  test\
		  Explorer\
		  Terminal\
		  Game3D

BUILD_ROOT ?= $(abspath ../build/user_prog)

.PHONY: all libhx86 $(SUBDIRS) clean

all: libhx86 $(SUBDIRS)

# Build the library first
libhx86:
	@echo ""
	@echo "--- Building libhx86 Library ---"
	@$(MAKE) -C libhx86 BUILD_DIR=$(BUILD_ROOT)/libhx86

# Build the apps
$(SUBDIRS): libhx86
	@echo ""
	@echo "--- Building $@ ---"
	@$(MAKE) -C $@ BUILD_DIR=$(BUILD_ROOT)/$@ LIBHX86_BUILD_DIR=$(BUILD_ROOT)/libhx86

clean:
	@$(MAKE) -C libhx86 clean BUILD_DIR=$(BUILD_ROOT)/libhx86
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean BUILD_DIR=$(BUILD_ROOT)/$$dir; \
	done