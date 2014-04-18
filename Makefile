SHELL = /bin/bash

BUILD_PATH = build
CXX_SRC_EXT = cc
SRC_PATH = src
TEST_SRC_PATH = test

INCLUDES = -I $(SRC_PATH)

TEST_INCLUDES = $(INCLUDES) -I $(TEST_SRC_PATH) -I test/ext/Catch/single_include

LIB_SRC = $(shell find $(SRC_PATH)/ -name '*.$(CXX_SRC_EXT)')
LIB_OBJ = $(LIB_SRC:$(SRC_PATH)/%.$(CXX_SRC_EXT)=$(BUILD_PATH)/%.o)
LIB_DEPS = $(LIB_OBJ:.o=.d)

TEST_SRC = $(shell find $(TEST_SRC_PATH)/ -name '*.$(CXX_SRC_EXT)')
TEST_OBJ = $(TEST_SRC:$(TEST_SRC_PATH)/%.$(CXX_SRC_EXT)=$(BUILD_PATH)/%.o)
TEST_DEPS = $(TEST_OBJ:.o=.d)

.PHONY: all
all: libmarkdownparser.a test-libmarkdownparser

libmarkdownparser.a: $(LIB_OBJ)
	$(AR) rcs $(BUILD_PATH)/libmarkdownparser.a $^

test-libmarkdownparser: libmarkdownparser.a $(TEST_OBJ)
	$(CXX) $(TEST_OBJ) $(BUILD_PATH)/libmarkdownparser.a $(LDFLAGS) -o $(BUILD_PATH)/$@

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

$(LIB_OBJ): | $(BUILD_PATH)

$(BUILD_PATH):
	mkdir -p $(BUILD_PATH)
