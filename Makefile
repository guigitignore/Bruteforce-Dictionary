CC=gcc
SRC=main.c dictionary.c hash.c parser.c array.c
OUT=main
CFLAGS=-Wall -g
LDFLAGS=$(shell pkg-config --libs openssl) -lpthread

all:
	@$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS) -O5
	@./$(OUT)