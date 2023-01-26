CC = gcc
CFLAGS = -g -Wall -Wextra
LOADLIBES=-lm -fopenmp

EXECUTABLES = Pooling

.PHONY: all
all: $(EXECUTABLES)

$(EXECUTABLES): util.o

util.o: util.h

.PHONY: clean
clean:
	rm -vf *.o $(EXECUTABLES)
