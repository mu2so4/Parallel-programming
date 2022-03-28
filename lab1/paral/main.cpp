#include <mpi.h>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include "linears.h"

int rank;
int processCount;
int workZoneLeft;
int *offsets;
int *subSizes;
int subSize;
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

	double epsilon = atof(argv[1]), *pre_sub_koefs, *pre_b;
	int N;

	if(!rank) {
		std::ifstream in(argv[2]);
		in >> N;
        MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
        workZoneLeft = N * rank / processCount;
	    subSize = N * (rank + 1) / processCount - workZoneLeft;

		pre_b = new double[N];
		for(int index = 0; index < N; index++) {
			in >> pre_b[index];
			if(in.eof()) {
				std::cerr << "reached the end of file too early\n";
				throw std::exception();
			}
		}
        MPI_Bcast(pre_b, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        double *pre_koefs = new double[N * N];
		for(int index = 0; index < N * N; index++) {
			in >> pre_koefs[index];
			if(in.eof()) {
				std::cerr << "reached the end of file too early\n";
				throw std::exception();
			}
		}
		in.close();
        pre_sub_koefs = new double[N * subSize];
        int *mat_offsets = new int[processCount], *mat_subSizes = new int[processCount];
        mat_offsets[0] = 0;
        for(int index = 0; index < processCount - 1; index++) {
            mat_offsets[index + 1] = (N * (index + 1) / processCount) * N;
            mat_subSizes[index] = mat_offsets[index + 1] - mat_offsets[index];
        }
        mat_subSizes[processCount - 1] = N * N - mat_offsets[processCount - 1];
        MPI_Scatterv(pre_koefs, mat_subSizes, mat_offsets, MPI_DOUBLE, pre_sub_koefs, subSize * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        delete[] mat_offsets;
        delete[] mat_subSizes;
        delete[] pre_koefs;
	}
    else {
	    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
        workZoneLeft = N * rank / processCount;
	    subSize = N * (rank + 1) / processCount - workZoneLeft;

        pre_b = new double[N];
        MPI_Bcast(pre_b, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        pre_sub_koefs = new double[N * subSize];
        MPI_Scatterv(nullptr, nullptr, nullptr, MPI_DOUBLE, pre_sub_koefs, subSize * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    offsets = new int[processCount]; subSizes = new int[processCount];
    offsets[0] = 0; subSizes[0] = N / processCount;
   	for(int index = 1; index < processCount; index++) {
    	subSizes[index] = N * (index + 1) / processCount - N * index / processCount;
   		offsets[index] = offsets[index - 1] + subSizes[index - 1];
   	}

	Vector b(pre_b, N);
	Matrix koefs(pre_sub_koefs, subSize, N);

	double *arrx0 = new double[N];
	for(int index = 0; index < N; index++)
		arrx0[index] = 1;
	Vector result(arrx0, N), rest = b - koefs * result, z = rest;

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
    if(!rank) {
        std::cerr << "It took " << endTime - startTime << " seconds\n";
    }
    delete[] offsets; delete[] subSizes;

	MPI_Finalize();
	return 0;
}
