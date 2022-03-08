CC=g++
CFLAGS=-O2 -c -Wall -std=c++14

all: non-p sle-generator check

non-p: main.o series.o
	$(CC) main.o series.o -o non-p.exe

main.o: main.cpp linears.h
	$(CC) $(CFLAGS) main.cpp -o main.o

series.o: series.cpp linears.h
	$(CC) $(CFLAGS) series.cpp -o series.o

sle-generator: gen_sle.o
	$(CC) gen_sle.o -o sle-generator.exe

gen_sle.o: gen_sle.cpp
	$(CC) $(CFLAGS) gen_sle.cpp -o gen_sle.o

check: check.o simple.o
	$(CC) check.o simple.o -o check.exe

check.o: check.cpp linears.h
	$(CC) $(CFLAGS) check.cpp -o check.o

clean:
	rm -rf *.o 
