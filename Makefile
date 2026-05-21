CC = gcc

CFLAGS = -Wall -Wextra -pthread -Iinclude

DEBUG_FLAGS = -g -fsanitize=thread

SRC = src/main.c \
      src/bank.c \
      src/transaction.c \
      src/timer.c \
      src/lock_mgr.c \
      src/buffer_pool.c \
      src/metrics.c \
      src/utils.c

TARGET = bankdb

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

debug:
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

run: all
	./$(TARGET)