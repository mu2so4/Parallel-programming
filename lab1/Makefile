CC=g++
CFLAGS=-O2 -c -Wall

all: non_p sle_generator check

non_p: main.o simple.o
	$(CC) main.o simple.o -o non_p

main.o: main.cpp linears.h
	$(CC) $(CFLAGS) main.cpp -o main.o

simple.o: simple.cpp linears.h
	$(CC) $(CFLAGS) simple.cpp -o simple.o

sle_generator: gen_sle.o
	$(CC) gen_sle.o -o sle-generator

gen_sle.o: gen_sle.cpp
	$(CC) $(CFLAGS) gen_sle.cpp -o gen_sle.o

check: check.o simple.o
	$(CC) check.o simple.o -o check

check.o: check.cpp linears.h
	$(CC) $(CFLAGS) check.cpp -o check.o

clean:
	rm -rf *.o 
