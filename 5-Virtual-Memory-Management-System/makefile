CC=g++
CFLAGS=-DLOGS

all: libmemlab.a demo1 

libmemlab.a: memlab.o
	ar -rcs libmemlab.a memlab.o

memlab.o: memlab.cc memlab.h
	$(CC) $(CFLAGS) -c memlab.cc -lpthread

demo1: demo1.o libmemlab.a
	$(CC) $(CFLAGS) -o demo1 demo1.o -L. -lmemlab -lpthread

demo1.o: demo1.cpp
	$(CC) $(CFLAGS) -c demo1.cpp