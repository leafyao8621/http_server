CC = gcc
SRC = $(wildcard src/*/*.c) $(wildcard src/*/*/*.c) src/main.c
OBJ = $(SRC:.c=.o)
BIN = main
CFLAGS = -Wall -Wextra -Werror -pedantic

%.o: %.c
	$(CC) $(CFLAGS) -g -c $< -o $@

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) -pthread

.PHONY: clean
clean:
	@rm $(OBJ)
	@rm $(BIN)
