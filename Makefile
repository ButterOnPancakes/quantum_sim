CC = gcc
CFLAGS = -W -Wall -pedantic #-fopenmp 
LDFLAGS = -lm #-fopenmp -lpthread

BIN_DIR = bin

#Compile all objects
$(BIN_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

BUILDER = $(shell find builder -name "*.c")
SIMULATOR = $(shell find simulator -name "*.c")
UTILS = $(shell find utils -name "*.c")

SOURCES = $(BUILDER) $(SIMULATOR) $(UTILS)

OBJECTS = $(patsubst %.c,$(BIN_DIR)/%.o,$(SOURCES))

EX_SRC = $(shell find examples -name "*.c")
EX_BIN = $(patsubst %.c,$(BIN_DIR)/%,$(EX_SRC))

$(EX_BIN): $(BIN_DIR)/%: $(BIN_DIR)/%.o $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

all:
	@$(MAKE) $(EX_BIN)

clean:
	rm -rf $(BIN_DIR)