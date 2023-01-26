CC = mpicc
CFLAGS = -g -Wall -Wextra
LOADLIBES=-lm

EXECUTABLES = Pooling

.PHONY: all
all: $(EXECUTABLES)

$(EXECUTABLES): util.o

util.o: util.h

.PHONY: clean
clean:
	rm -vf *.o $(EXECUTABLES)
run:
	mpirun --oversubscribe -np 2 Pooling ../input/test2.bmp test2-MPI.bmp

# mpirun -np 4 Pooling ../input/test10.bmp test10-MPI.bmp