CC = gcc
CFLAGS = -Wall -g
SRC_DIR = src
TEST_FILE = $(SRC_DIR)/test_malloc.c
MY_MALLOC = $(SRC_DIR)/my_malloc.c
FORMAT_FILES = $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*.h)
BIN_DIR = bin

# Create the bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

all: $(BIN_DIR) test_system test_custom

# Format the code
format:
	clang-format -i $(FORMAT_FILES)

# Compile test_system with system malloc/free
test_system: $(BIN_DIR) $(TEST_FILE)
	$(CC) $(CFLAGS) -DUSE_SYSTEM_ALLOCATOR -o $(BIN_DIR)/test_system $(TEST_FILE)

# Compile test_custom with your custom malloc/free
test_custom: $(BIN_DIR) $(TEST_FILE) $(MY_MALLOC)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/test_custom $(TEST_FILE) $(MY_MALLOC) -pthread

clean:
	rm -f $(BIN_DIR)/test_system $(BIN_DIR)/test_custom
