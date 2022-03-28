#include <mpi.h>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include "linears.h"

int rank;
int processCount;
int *offsets;
int *subSizes;
double endTime;

int main(int argc, char** argv) {
	if(argc != 3) {
		std::cout << "Wrong input\n";
		return 1;
	}
	MPI_Init(&argc, &argv);
    double startTime = MPI_Wtime();
	MPI_Comm_size(MPI_COMM_WORLD, &processCount);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	double epsilon = atof(argv[1]);
	int N, subSize;
	double *pre_sub_b, *pre_sub_koefs;

	if(!rank) {
		std::ifstream in(argv[2]);
		in >> N;
        MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
	    subSize = N * (rank + 1) / processCount - N * rank / processCount;

		double *pre_b = new double[N];
		for(int index = 0; index < N; index++) {
			in >> pre_b[index];
			if(in.eof()) {
				std::cerr << "reached the end of file too early\n";
				throw std::exception();
			}
		}
        pre_sub_b = new double[subSize];

        offsets = new int[processCount]; subSizes = new int[processCount];
        offsets[0] = 0; subSizes[0] = N / processCount;
        for(int index = 1; index < processCount; index++) {
            subSizes[index] = N * (index + 1) / processCount - N * index / processCount;
            offsets[index] = offsets[index - 1] + subSizes[index - 1];
        }

        MPI_Scatterv(pre_b, subSizes, offsets, MPI_DOUBLE, pre_sub_b, subSize, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        double *pre_koefs = new double[N * N];
		for(int index = 0; index < N * N; index++) {
			in >> pre_koefs[index];
			if(in.eof()) {
				std::cerr << "reached the end of file too early\n";
				throw std::exception();
			}
		}
		in.close();

        pre_sub_koefs = new double[subSize * N];
        for(int index = 0; index < N; index++) {
            MPI_Scatterv(pre_koefs + N * index, subSizes, offsets, MPI_DOUBLE, pre_sub_koefs + subSize * index, subSize, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        }
        delete[] pre_b;
        delete[] pre_koefs;
	}
    else {
        MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
        subSize = N * (rank + 1) / processCount - N * rank / processCount;

        pre_sub_b = new double[subSize];
        MPI_Scatterv(nullptr, nullptr, nullptr, MPI_DOUBLE, pre_sub_b, subSize, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        pre_sub_koefs = new double[subSize * N];
        for(int index = 0; index < N; index++) {
            MPI_Scatterv(nullptr, nullptr, nullptr, MPI_DOUBLE, pre_sub_koefs + subSize * index, subSize, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        }
    }
    

	Vector b(pre_sub_b, N, subSize);
	Matrix koefs(pre_sub_koefs, N, subSize);

	double *arrx0 = new double[subSize];
	for(int index = 0; index < subSize; index++)
		arrx0[index] = 1;
	Vector result(arrx0, N, subSize), rest = b - koefs * result, z = rest;

	double control = b.squareNorm() * epsilon * epsilon, r2 = rest.squareNorm();
	for(int iter = 0; iter < 10000 && r2 >= control; iter++) {
		Vector z_1 = koefs * z;
		double alpha = r2 / Vector::dotProduction(z_1, z);
		Vector new_rest = rest - alpha * z_1;
		double nr2 = new_rest.squareNorm(), beta = nr2 / r2;
		result = result + alpha * z;
		z = new_rest + beta * z;
		rest = new_rest;
        r2 = nr2;
	}
	result.print(std::cout);
    if(!rank)
        std::cerr << "It took " << endTime - startTime << " seconds\n";
    delete[] offsets; delete[] subSizes;

	MPI_Finalize();
	return 0;
}
