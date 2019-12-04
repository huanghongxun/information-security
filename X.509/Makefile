CC=gcc
CFLAGS=-g -Wall -Wextra
INC_DIR=include
SRC_DIR=src
BIN_DIR=bin
OBJ_DIR=obj

SOURCE_FILES=$(shell find $(SRC_DIR) -name '*.c')
HEADER_FILES=$(shell find $(SRC_DIR) -name '*.c')
OBJS=$(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCE_FILES))

$(BIN_DIR)/x509: $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -Iinclude $^ -o $@ -lm

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADER_FILES)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -Iinclude -c -o $@ $<

clean:
	@rm -rf $(OBJ_DIR)
	@rm -rf $(BIN_DIR)