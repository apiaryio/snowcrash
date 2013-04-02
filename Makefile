-include config.mk

BUILDTYPE ?= Release
BUILD_DIR ?= build
PYTHON ?= python
GYP ?= gyp

# Default to verbose builds
V ?= 1

# Targets
all: snowcrash

.PHONY: snowcrash

snowcrash: $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR) V=$(V)

$(BUILD_DIR)/Makefile:
	$(GYP) -f make --generator-output $(BUILD_DIR) --depth=.

clean:
	rm -rf $(BUILD_DIR)/out

distclean:
	rm -rf build
	rm -f config.mk

test: all
	$(BUILD_DIR)/out/Default/test-snowcrash

.PHONY: snowcrash clean distclean test
