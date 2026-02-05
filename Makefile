CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET_DIR = x64-linux
TARGET = $(TARGET_DIR)/MemoryPageCommitTest_linux
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
    @mkdir -p $(TARGET_DIR)
    $(CC) $(CFLAGS) -o $@ $^

clean:
    rm -rf $(TARGET_DIR)

.PHONY: all clean
