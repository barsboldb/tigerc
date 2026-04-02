CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Iinclude
DBGFLAGS = $(CFLAGS) -g -DDEBUG

BUILD_DIR = build
SRC_DIR   = src
TEST_DIR  = tests

# Collect all source files (add new subdirs here as you build each phase)
SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

TARGET = $(BUILD_DIR)/tigerc

.PHONY: all build debug test clean

# Default
all: build

# --- Build ---
build: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# --- Debug build (symbols + DEBUG macro) ---
debug: CFLAGS := $(DBGFLAGS)
debug: clean build

# --- Test ---
# All test files + test_main.c compiled into a single binary.
TEST_SRCS    = $(shell find $(TEST_DIR) -name '*.c')
NON_MAIN_OBJS = $(filter-out $(BUILD_DIR)/main.o, $(OBJS))
TEST_BIN     = $(BUILD_DIR)/test_runner

test: $(NON_MAIN_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SRCS) $(NON_MAIN_OBJS) -o $(TEST_BIN)
	$(TEST_BIN)

# --- Run the compiler on a tiger file ---
# Usage: make run FILE=examples/hello.tig
run: build
	$(TARGET) $(FILE)

# --- Clean ---
clean:
	rm -rf $(BUILD_DIR)
