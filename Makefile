CC=gcc
SRC=main.c dictionary.c hash.c parser.c
OUT=main
CFLAGS=-Wall -g
LDFLAGS=$(shell pkg-config --libs openssl)

all:
	@$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)
	@./$(OUT)