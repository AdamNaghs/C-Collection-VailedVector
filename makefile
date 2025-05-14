CC = gcc
CFLAGS = -ansi
SRC = ./tests/main.c ./source/vector.c
OUT = test

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
