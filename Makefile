CC = gcc

CFLAGS = -Iinclude
CFLAGS += -pedantic
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wno-gnu-statement-expression
CFLAGS += -std=c99
CFLAGS += -DDEBUG

LDFLAGS = -L/usr/local/lib

SOURCES = src/main.c src/chip8.c src/keypad.c src/display.c src/debugger.c

HEADERS = include/chip8.h
HEADERS += include/cpu_def.h
HEADERS += include/status_code.h
HEADERS += include/keypad.h
HEADERS += include/display.h
HEADERS += include/logging.h
HEADERS += include/debugger.h

LIBS = -lSDL2
OBJS = objects/main.o objects/chip8.o objects/keypad.o objects/display.o objects/debugger.o

all: bin/chip8_emu.out

bin/chip8_emu.out: $(OBJS) $(HEADERS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

objects/%.o: src/%.c
	@mkdir -p objects
	$(CC) -c $< $(CFLAGS) -o$@

clean:
	rm -rf bin objects