CC=gcc
CFLAGS=-O3 -Wall
DFLAGS=-Og -Wall

OBJECTS=src/instance.o src/main.o src/optimization.o src/timer.o src/utilities.o

.PHONY: clean

all: lop

lop: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o lop

debug: $(OBJECTS)
		$(CC) $(DFLAGS) $(OBJECTS) -o lop



clean:
	rm -f src/*~ src/*.o lop
