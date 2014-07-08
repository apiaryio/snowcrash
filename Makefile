-include config.mk

BUILDTYPE ?= Release
BUILD_DIR ?= ./build
PYTHON ?= python
GYP ?= ./tools/gyp/gyp
DESTDIR ?= /usr/local/bin

# Default to verbose builds
V ?= 1

# Targets
all: libsnowcrash test-libsnowcrash snowcrash

.PHONY: libsnowcrash test-libsnowcrash snowcrash

libsnowcrash: config.gypi $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR) V=$(V) libsnowcrash

test-libsnowcrash: config.gypi $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR) V=$(V) test-libsnowcrash
	mkdir -p ./bin
	cp -f $(BUILD_DIR)/out/$(BUILDTYPE)/test-libsnowcrash ./bin/test-libsnowcrash

perf-libsnowcrash: config.gypi $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR) V=$(V) perf-libsnowcrash
	mkdir -p ./bin
	cp -f $(BUILD_DIR)/out/$(BUILDTYPE)/perf-libsnowcrash ./bin/perf-libsnowcrash

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

test: test-libsnowcrash snowcrash
	$(BUILD_DIR)/out/$(BUILDTYPE)/test-libsnowcrash

ifdef INTEGRATION_TESTS
	bundle exec cucumber
endif

perf: perf-libsnowcrash
	$(BUILD_DIR)/out/$(BUILDTYPE)/perf-libsnowcrash ./test/performance/fixtures/fixture-1.md

install: snowcrash
	cp -f $(BUILD_DIR)/out/$(BUILDTYPE)/snowcrash $(DESTDIR)/snowcrash

.PHONY: libsnowcrash test-libsnowcrash perf-libsnowcrash snowcrash clean distclean test
