CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./src
SRC = src/main.c src/cpu.c src/memory.c src/ppu.c src/timer.c src/input.c src/cartridge.c
OBJ = $(SRC:.c=.o)
TARGET = gameboy-emulator

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean