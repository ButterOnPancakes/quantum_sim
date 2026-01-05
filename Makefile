# Compiler and flags
CC = gcc
CFLAGS = -Wall -fopenmp -march=native #-g -fsanitize=address
LDFLAGS = -lm

# Directories
BIN_DIR = bin
BUILDER_DIR = builder
SIMULATOR_DIR = simulator
UTILS_DIR = utils
EXAMPLES_DIR = examples

SOURCES_DIR = $(BUILDER_DIR) $(SIMULATOR_DIR) $(UTILS_DIR)

SOURCES = $(shell find $(SOURCES_DIR) -name "*.c")
OBJECTS = $(SOURCES:%.c=$(BIN_DIR)/%.o)

EXAMPLES = $(shell find $(EXAMPLES_DIR) -name "*.c")

# Pattern rule for object files
$(BIN_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for executables
$(BIN_DIR)/$(EXAMPLES_DIR)/%: $(BIN_DIR)/$(EXAMPLES_DIR)/%.o $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Build all example executables
all: $(patsubst $(EXAMPLES_DIR)/%.c,$(BIN_DIR)/$(EXAMPLES_DIR)/%,$(EXAMPLES))

# Clean
clean:
	rm -rf $(BIN_DIR)/*

# Individual example targets (for convenience)
test_computer: $(BIN_DIR)/$(EXAMPLES_DIR)/computer_test
	./$<

test_grover: $(BIN_DIR)/$(EXAMPLES_DIR)/grover
	./$<
	
test_qft: $(BIN_DIR)/$(EXAMPLES_DIR)/qft
	./$<

print-sources:
	@echo "SOURCES: $(SOURCES)"

print-objects:
	@echo "OBJECTS: $(OBJECTS)"