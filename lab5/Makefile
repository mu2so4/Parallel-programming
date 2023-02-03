CXX=icpc
CXXP=mpiicpc
CFLAGS=-c -Wall -Wextra -std=c++14 -O3 -mt_mpi

MAIN_SOURCE=main
TASK_SOURCE=threads

EXEC_EXT=out

all: $(MAIN_SOURCE).$(EXEC_EXT)

$(MAIN_SOURCE): $(MAIN_SOURCE).o $(TASK_SOURCE).o
	$(CXXP) $(MAIN_SOURCE).o $(TASK_SOURCE).o -o $(MAIN_SOURCE).$(EXEC_EXT)

$(MAIN_SOURCE).o: $(MAIN_SOURCE).cpp $(TASK_SOURCE).h
	$(CXXP) $(CFLAGS) $(MPIFLAGS) $(MAIN_SOURCE).cpp -o $(MAIN_SOURCE).o

$(TASK_SOURCE).o: $(TASK_SOURCE).cpp $(TASK_SOURCE).h
	$(CXXP) $(CFLAGS) $(TASK_SOURCE).cpp -o $(TASK_SOURCE).o

clean:
	rm -rf *.o
