# Default Shell
SHELL = /bin/bash

# Default build path
BUILD_PATH = build

# Default C++ extension
CXX_SRC_EXT = cc

# Default C extension
C_SRC_EXT = c

# Markdown parser source directory
SRC_PATH = src

# Test source directory
TEST_SRC_PATH = test

# Sundown include paths
SUNDOWN_INCLUDES = -Iext/sundown/src/ -Iext/sundown/html/

# Markdown parser include paths
INCLUDES = -I$(SRC_PATH) $(SUNDOWN_INCLUDES)

# Test include paths
TEST_INCLUDES = $(INCLUDES) -I$(TEST_SRC_PATH) -Itest/ext/Catch/single_include

# Markdown parser objects
LIB_SRC = $(shell find $(SRC_PATH)/ -name '*.$(CXX_SRC_EXT)')
LIB_OBJ = $(LIB_SRC:$(SRC_PATH)/%.$(CXX_SRC_EXT)=$(BUILD_PATH)/%.o)
LIB_DEPS = $(LIB_OBJ:.o=.d)

# Sundown library objects
SUNDOWN_SRC=\
        ext/sundown/src/markdown.c \
        ext/sundown/src/stack.c \
        ext/sundown/src/buffer.c \
        ext/sundown/src/autolink.c \
        ext/sundown/src/src_map.c \
        ext/sundown/html/html.c \
        ext/sundown/html/html_smartypants.c \
        ext/sundown/html/houdini_html_e.c \
        ext/sundown/html/houdini_href_e.c
SUNDOWN_OBJ = $(addprefix $(BUILD_PATH)/,$(addsuffix .o,$(basename $(SUNDOWN_SRC))))

# Markdown parser tests object
TEST_SRC = $(shell find $(TEST_SRC_PATH)/ -name '*.$(CXX_SRC_EXT)')
TEST_OBJ = $(TEST_SRC:$(TEST_SRC_PATH)/%.$(CXX_SRC_EXT)=$(BUILD_PATH)/%.o)
TEST_DEPS = $(TEST_OBJ:.o=.d)

.PHONY: all
all: libmarkdownparser.a test-libmarkdownparser libsundown.a	

libsundown.a: $(SUNDOWN_OBJ) 
	$(AR) rcs $(BUILD_PATH)/libsundown.a $^

libmarkdownparser.a: $(LIB_OBJ)
	$(AR) rcs $(BUILD_PATH)/libmarkdownparser.a $^

test-libmarkdownparser: libmarkdownparser.a libsundown.a $(TEST_OBJ)
	$(CXX) $(TEST_OBJ) $(BUILD_PATH)/libmarkdownparser.a $(BUILD_PATH)/libsundown.a $(LDFLAGS) -o $(BUILD_PATH)/$@

.PHONY: test
test: test-libmarkdownparser
	$(BUILD_PATH)/test-libmarkdownparser

.PHONY: clean
clean:
	$(RM) -r build
	$(RM) -r bin

$(BUILD_PATH)/%.o: $(SRC_PATH)/%.$(CXX_SRC_EXT)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@

$(BUILD_PATH)/%.o: $(TEST_SRC_PATH)/%.$(CXX_SRC_EXT)
	$(CXX) $(CXXFLAGS) $(TEST_INCLUDES) -MP -MMD -c $< -o $@	

$(BUILD_PATH)/%.o: %.$(C_SRC_EXT)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@	

$(LIB_OBJ): | $(BUILD_PATH)
$(SUNDOWN_OBJ): | $(BUILD_PATH)

$(BUILD_PATH):
	mkdir -p $(BUILD_PATH)
	mkdir -p $(BUILD_PATH)/ext/sundown/src/
	mkdir -p $(BUILD_PATH)/ext/sundown/html/
