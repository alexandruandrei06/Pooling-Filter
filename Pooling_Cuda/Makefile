all: Pooling

Pooling: util.o
	nvcc -g -arch=sm_35 -rdc=true Pooling.cu util.o -o Pooling -lm

util.o: util.h
	nvcc -g util.cpp -c

clean:
	rm -f Pooling *.o