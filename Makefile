CC = gcc
CFLAGS = -Iinclude -pedantic -Wall -Wextra -Wno-gnu-statement-expression -std=c99
LDFLAGS = -L/usr/local/lib

SOURCES = src/main.c
SOURCES += src/chip8.c
SOURCES += src/keypad.c
SOURCES += src/display.c
SOURCES += src/timer.c
SOURCES += src/audio.c

HEADERS = include/chip8.h
HEADERS += include/cpu_def.h
HEADERS += include/status_code.h
HEADERS += include/keypad.h include/display.h
HEADERS += include/logging.h
HEADERS += include/timer.h
HEADERS += include/audio.h

LIBS = -lSDL2
OBJS = objects/main.o objects/chip8.o objects/keypad.o objects/display.o objects/timer.o objects/audio.o

all: bin/chip8_emu.out

bin/chip8_emu.out: $(OBJS) $(HEADERS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

objects/%.o: src/%.c
	@mkdir -p objects
	$(CC) -c $< $(CFLAGS) -o$@

clean:
	rm -rf bin objects