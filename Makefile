CC=gcc
SRC=main.c dictionary.c hash.c
OUT=main
CFLAGS=-Wall
LDFLAGS=$(shell pkg-config --libs openssl)

all:
	@$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)
	@./$(OUT) 