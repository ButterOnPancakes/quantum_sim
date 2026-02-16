# Compiler and flags
CC = gcc
CFLAGS = -Wall -fopenmp -march=native #-g -fsanitize=address
LDFLAGS = -lm -fopenmp
LDFLAGS_GUI = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -fopenmp

# The versions you want to compile
VERSIONS = naive_version opti_version emms_version dag_version

# Output Directory
BIN_DIR = bin

# --- 1. Generic Rule for Object Files ---
# This matches any .c file and places the .o file in bin/, preserving directory structure
# e.g., naive_version/utils/foo.c -> bin/naive_version/utils/foo.o
$(BIN_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# --- 2. The Build Template (Macro) ---
# This function generates the specific variables and rules for one version.
# $(1) will be replaced by the version name (e.g., naive_version)
define VERSION_TEMPLATE

# A. Discovery: Find sources specific to this version
# Note: We look specifically inside the folder named $(1)
$(1)_BUILDER    := $$(shell find $(1)/builder -name "*.c")
$(1)_SIMULATOR  := $$(shell find $(1)/simulator -name "*.c")
$(1)_UTILS      := $$(shell find $(1)/utils -name "*.c")
$(1)_SOURCES    := $$($(1)_BUILDER) $$($(1)_SIMULATOR) $$($(1)_UTILS)

# B. Calculate Objects: Convert .c paths to .o paths inside bin/
$(1)_OBJECTS    := $$(patsubst %.c,$(BIN_DIR)/%.o,$$($(1)_SOURCES))

# C. Examples: Find and calculate paths
$(1)_EX_SRC     := $$(shell find $(1)/examples -name "*.c")
$(1)_EX_BIN     := $$(patsubst %.c,$(BIN_DIR)/%,$$($(1)_EX_SRC))


# D. GUI: Find and calculate paths
#$(1)_GUI_SRC    := $$(shell find $(1)/gui -name "*.c")
#$(1)_GUI_OBJ    := $$(patsubst %.c,$(BIN_DIR)/%.o,$$($(1)_GUI_SRC))
#$(1)_GUI_APP    := $(BIN_DIR)/$(1)/gui_app

# --- Rules ---

# Rule to build Examples for this specific version
# It depends on its specific object file and the version's library objects
$$($(1)_EX_BIN): $(BIN_DIR)/%: $(BIN_DIR)/%.o $$($(1)_OBJECTS)
	@mkdir -p $$(dir $$@)
	$(CC) $(CFLAGS) -o $$@ $$^ $(LDFLAGS)

# Rule to build GUI for this specific version
#$$($(1)_GUI_APP): $$($(1)_GUI_OBJ) $$($(1)_OBJECTS)
#	@mkdir -p $$(dir $$@)
#	$(CC) $(CFLAGS) -o $$@ $$^ $(LDFLAGS_GUI)

# Add these targets to the global list so 'make all' sees them
ALL_TARGETS += $$($(1)_EX_BIN) $$($(1)_GUI_APP)

endef

# --- 3. The Loop ---
# Iterate over VERSIONS and call the template for each one
$(foreach v,$(VERSIONS),$(eval $(call VERSION_TEMPLATE,$(v))))

# --- 4. Main Targets ---

all: $(ALL_TARGETS)

clean:
	rm -rf $(BIN_DIR)

print-versions:
	@echo "Building versions: $(VERSIONS)"

# Debug helper to see what will be built
print-targets:
	@echo "Targets: $(ALL_TARGETS)"