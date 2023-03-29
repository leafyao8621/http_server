CC = gcc
SRC = $(wildcard src/*/*.c) $(wildcard src/*/*/*.c)
OBJ = $(SRC:.c=.o)
LIB = lib/libhttpserver.so
BIN = test
CFLAGS = -Wall -Wextra -Werror -pedantic -fPIC

%.o: %.c
	$(CC) $(CFLAGS) -O3 -c $< -o $@

$(LIB): $(OBJ)
	$(CC) $(OBJ) -shared -o $(LIB) -lcontainers -lhttputil -ljson

$(BIN): $(LIB)
	@cp src/util/errcode/errcode.h include/http_server/
	$(CC) src/test.c -o $(BIN) -pthread -Llib -lcontainers -lhttputil -ljson -lhttpserver -Iinclude -Wl,-rpath,lib

.PHONY: clean install
clean:
	@rm $(OBJ)
	@rm $(LIB)
	@rm $(BIN)
install:
	@cp -r include/http_server ~/.local/include
	@cp $(LIB) ~/.local/lib
