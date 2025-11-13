# Compiler and flags
CC = gcc
CFLAGS = -g -Wall -fsanitize=address -fopenmp
LDFLAGS = -lm

# Directories
BIN_DIR = bin
COMPUTER_DIR = $(BIN_DIR)/computer
EXAMPLES_DIR = $(BIN_DIR)/examples
UTILS_DIR = $(BIN_DIR)/utils
LISTS_DIR = $(UTILS_DIR)/lists
MATRICES_DIR = $(BIN_DIR)/matrices

# Common object files
COMMON_OBJS = $(COMPUTER_DIR)/naive/circuit.o $(LISTS_DIR)/list.o $(UTILS_DIR)/utils.o $(MATRICES_DIR)/naive/matrices.o

# Targets
TARGETS = $(EXAMPLES_DIR)/computer_test $(EXAMPLES_DIR)/grover

# Default target
all: $(TARGETS)

# Pattern rule for object files
$(BIN_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for executables
$(COMPUTER_DIR)/%: $(COMPUTER_DIR)/%.o $(COMMON_OBJS)
	@mkdir -p $(COMPUTER_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Pattern rule for executables
$(EXAMPLES_DIR)/%: $(EXAMPLES_DIR)/%.o $(COMMON_OBJS)
	@mkdir -p $(EXAMPLES_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Test targets
test_computer: $(EXAMPLES_DIR)/computer_test
	./$<

grover: $(EXAMPLES_DIR)/grover
	./$<

# Clean
clean:
	rm -rf $(BIN_DIR)/*

# Phony targets
.PHONY: all test_computer grover clean