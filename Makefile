CC = gcc
CFLAGS = -Iinclude -pedantic -Wall -Wextra -Wno-gnu-statement-expression -std=c99
LDFLAGS = -L/usr/local/lib

SOURCES = src/main.c src/chip8.c src/keypad.c src/display.c
HEADERS = include/chip8.h include/cpu_def.h include/status_code.h include/keypad.h include/display.h include/logging.h
LIBS = -lSDL2
OBJS = objects/main.o objects/chip8.o objects/keypad.o objects/display.o

all: bin/chip8_emu.out

bin/chip8_emu.out: $(OBJS) $(HEADERS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

objects/%.o: src/%.c
	@mkdir -p objects
	$(CC) -c $< $(CFLAGS) -o$@

clean:
	rm -rf bin objects