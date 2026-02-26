CC = gcc
CFLAGS = -fopenmp -W -Wall -pedantic
LDFLAGS = -fopenmp -lm -lpthread
LDFLAGSGUI = -fopenmp -lm -lpthread -lraylib

BIN_DIR = bin

#Compile all objects
$(BIN_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

BUILDER = $(shell find builder -name "*.c")
SIMULATOR = $(shell find simulator -name "*.c")
UTILS = $(shell find utils -name "*.c")
LOGS = $(shell find logs -name "*.c")

EXAMPLES = $(shell find examples -name "*.c")
GUI = $(shell find gui -name "*.c")

SOURCES = $(BUILDER) $(SIMULATOR) $(UTILS) $(LOGS)

OBJECTS = $(patsubst %.c,$(BIN_DIR)/%.o,$(SOURCES))

EX_BIN = $(patsubst %.c,$(BIN_DIR)/%,$(EXAMPLES))
GUI_BIN = $(patsubst %.c,$(BIN_DIR)/%,$(GUI))

$(EX_BIN): $(BIN_DIR)/%: $(BIN_DIR)/%.o $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(GUI_BIN): $(BIN_DIR)/%: $(BIN_DIR)/%.o $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGSGUI)

clean:
	rm -rf $(BIN_DIR)