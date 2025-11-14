# Compiler and flags
CC = gcc
CFLAGS = -g -Wall -fsanitize=address -fopenmp
LDFLAGS = -lm

# Directories
BIN_DIR = bin
BUILDER_DIR = $(BIN_DIR)/builder
EXAMPLES_DIR = $(BIN_DIR)/examples
SIMULATOR_DIR = $(BIN_DIR)/simulator
UTILS_DIR = $(BIN_DIR)/utils
LISTS_DIR = $(UTILS_DIR)/lists

# Common object files
COMMON_OBJS = $(BUILDER_DIR)/circuit.o \
	$(SIMULATOR_DIR)/naive/matrices.o \
	$(SIMULATOR_DIR)/naive/base_sim.o \
	$(LISTS_DIR)/list.o \
	$(UTILS_DIR)/utils.o \

# Targets
TARGETS = $(EXAMPLES_DIR)/computer_test $(EXAMPLES_DIR)/grover

# Default target
all: $(TARGETS)

# Pattern rule for object files
$(BIN_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for executables
$(BUILDER_DIR)/%: $(BUILDER_DIR)/%.o $(COMMON_OBJS)
	@mkdir -p $(BUILDER_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Pattern rule for executables
$(SIMULATOR_DIR)/%: $(SIMULATOR_DIR)/%.o $(COMMON_OBJS)
	@mkdir -p $(SIMULATOR_DIR)
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