-include config.mk

BUILDTYPE ?= Release
BUILD_DIR ?= ./build
PYTHON ?= python
GYP ?= ./tools/gyp/gyp

# Default to verbose builds
V ?= 1

# Targets
all: libsnowcrash test-snowcrash snowcrash

.PHONY: libsnowcrash test-snowcrash snowcrash

libsnowcrash: config.gypi $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR) V=$(V) libsnowcrash

test-snowcrash: config.gypi $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR) V=$(V) test-snowcrash
	mkdir -p ./bin
	cp -f $(BUILD_DIR)/out/$(BUILDTYPE)/test-snowcrash ./bin/test-snowcrash

perf-snowcrash: config.gypi $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR) V=$(V) perf-snowcrash
	mkdir -p ./bin
	cp -f $(BUILD_DIR)/out/$(BUILDTYPE)/perf-snowcrash ./bin/perf-snowcrash

snowcrash: config.gypi $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR) V=$(V) snowcrash
	mkdir -p ./bin
	cp -f $(BUILD_DIR)/out/$(BUILDTYPE)/snowcrash ./bin/snowcrash

config.gypi: configure
	$(PYTHON) ./configure

$(BUILD_DIR)/Makefile:
	$(GYP) -f make --generator-output $(BUILD_DIR) --depth=.

clean:
	rm -rf $(BUILD_DIR)/out
	rm -rf ./bin

distclean:
	rm -rf ./build
	rm -f ./config.mk
	rm -f ./config.gypi
	rm -rf ./bin	

test: test-snowcrash snowcrash
	$(BUILD_DIR)/out/$(BUILDTYPE)/test-snowcrash
	
ifdef CUCUMBER
	$(CUCUMBER)
endif

perf: perf-snowcrash
	$(BUILD_DIR)/out/$(BUILDTYPE)/perf-snowcrash ./test/performance/fixtures/fixture-1.md

install: snowcrash
	cp -f $(BUILD_DIR)/out/$(BUILDTYPE)/snowcrash /usr/local/bin/snowcrash

.PHONY: libsnowcrash test-snowcrash perf-snowcrash snowcrash clean distclean test
