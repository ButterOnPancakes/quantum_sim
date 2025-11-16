# Compiler and flags
CC = gcc
# -g -Wall -fsanitize=address 
CFLAGS = -fopenmp -march=native
LDFLAGS = -lm

# Directories
BIN_DIR = bin
EXCLUDED_DIRS = .venv .vscode benchmarks bin examples
BUILDER_DIR = $(BIN_DIR)/builder
EXAMPLES_DIR = $(BIN_DIR)/examples
SIMULATOR_DIR = $(BIN_DIR)/simulator
UTILS_DIR = $(BIN_DIR)/utils
LISTS_DIR = $(UTILS_DIR)/lists

# Find all source files recursively, excluding specified directories
SOURCES = $(shell find . -name "*.c" $(foreach dir,$(EXCLUDED_DIRS),-not -path "./$(dir)/*") $(foreach dir,$(EXCLUDED_DIRS),-not -path "./$(dir)"))
OBJECTS = $(SOURCES:%.c=$(BIN_DIR)/%.o)

# Find all executable source files in examples (those with main function)
EXEC_SOURCES = $(shell find examples -name "*.c")
EXEC_TARGETS = $(EXEC_SOURCES:examples/%.c=$(EXAMPLES_DIR)/%)

# Common object files (non-executable objects)
COMMON_OBJS = $(filter-out $(EXEC_SOURCES:%.c=$(BIN_DIR)/%.o), $(OBJECTS))

# Targets
TARGETS = $(EXEC_TARGETS)

# Default target
all: $(TARGETS)

# Pattern rule for object files
$(BIN_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for executables
$(EXAMPLES_DIR)/%: $(BIN_DIR)/examples/%.o $(COMMON_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Individual example targets (for convenience)
test_computer: $(EXAMPLES_DIR)/computer_test
	./$<

test_grover: $(EXAMPLES_DIR)/grover
	./$<

# Build all examples without running
build_examples: $(TARGETS)

# Clean
clean:
	rm -rf $(BIN_DIR)/*

# Phony targets
.PHONY: all build_examples computer_test grover clean

# Debug targets (useful for debugging the makefile)
print-sources:
	@echo "SOURCES: $(SOURCES)"

print-objects:
	@echo "OBJECTS: $(OBJECTS)"

print-exec-sources:
	@echo "EXEC_SOURCES: $(EXEC_SOURCES)"

print-exec-targets:
	@echo "EXEC_TARGETS: $(EXEC_TARGETS)"

print-common-objs:
	@echo "COMMON_OBJS: $(COMMON_OBJS)"

print-excluded:
	@echo "EXCLUDED_DIRS: $(EXCLUDED_DIRS)"