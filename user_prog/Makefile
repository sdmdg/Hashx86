SUBDIRS = MeMView\
		  test

.PHONY: all libhx86 $(SUBDIRS) clean

all: libhx86 $(SUBDIRS)

# Build the library first
libhx86:
	@echo ""
	@echo "--- Building libhx86 Library ---"
	@$(MAKE) -C libhx86

# Build the apps
$(SUBDIRS): libhx86
	@echo ""
	@echo "--- Building $@ ---"
	@$(MAKE) -C $@

clean:
	@$(MAKE) -C libhx86 clean
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done