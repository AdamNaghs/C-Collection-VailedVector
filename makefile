CC = gcc
CFLAGS = -ansi
SRC = ./tests/main.c ./source/vector.c
OUT = test.exe

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	del -f $(OUT)
