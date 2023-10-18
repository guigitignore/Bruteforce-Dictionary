CC=gcc
SRC=main.c dictionary.c hash.c parser.c
OUT=main
CFLAGS=-Wall -g
LDFLAGS=$(shell pkg-config --libs openssl) -lpthread

all:
	@$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)
	@./$(OUT)