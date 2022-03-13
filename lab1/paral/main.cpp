#include <fstream>
#include <cstdlib>
#include <iostream>
#include <mpi.h>
#include "linears.h"

int rank;
int processCount;
int workZoneLeft;
int workZoneRight;
int *offsets;
int *subSizes;

int main(int argc, char** argv) {
	if(argc != 3) {
		std::cout << "Wrong input\n";
		return 1;
	}
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &processCount);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	double epsilon = atof(argv[1]);
	int dim;
	double *pre_b = nullptr, *pre_koefs = nullptr;

	if(!rank) {
		std::ifstream in(argv[2]);
		in >> dim;
		pre_b = new double[dim]; pre_koefs = new double[dim * dim];
		for(int index = 0; index < dim; index++) {
			in >> pre_b[index];
			if(in.eof()) {
				std::cerr << "reached the end of file too early\n";
				throw std::exception();
			}
		}

		for(int index = 0; index < dim * dim; index++) {
			in >> pre_koefs[index];
			if(in.eof()) {
				std::cerr << "reached the end of file too early\n";
				throw std::exception();
			}
		}
		in.close();
	}
	
	MPI_Bcast(&dim, 1, MPI_INT, 0, MPI_COMM_WORLD);
	workZoneLeft = dim * rank / processCount;
	workZoneRight = dim * (rank + 1) / processCount;
	if(rank) {
		pre_b = new double[dim]; pre_koefs = new double[dim * dim];
	}
	else {
        	offsets = new int[processCount]; subSizes = new int[processCount];
	        offsets[0] = 0; subSizes[0] = dim / processCount;
        	for(int index = 1; index < processCount; index++) {
   	        	subSizes[index] = dim * (index + 1) / processCount - dim * index / processCount;
        		offsets[index] = offsets[index - 1] + subSizes[index - 1];
        	}
	}
	
	MPI_Bcast(pre_b, dim, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(pre_koefs, dim * dim, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	Vector b(pre_b, dim);
	Matrix koefs(pre_koefs, dim);
	delete[] pre_b;
	delete[] pre_koefs;

	double *arrx0 = new double[dim];
	for(int index = 0; index < dim; index++)
		arrx0[index] = 1;
	Vector result(arrx0, dim), rest = b - koefs * result, z = rest;
	delete[] arrx0;

	double control = b.squareNorm() * epsilon * epsilon;

	for(int iter = 0; iter < 10000 && rest.squareNorm() >= control; iter++) {
		Vector z_1 = koefs * z;
		double alpha = Vector::dotProduction(rest, rest) / Vector::dotProduction(z_1, z);
		Vector new_rest = rest - alpha * z_1;
		double beta = Vector::dotProduction(new_rest, new_rest) / Vector::dotProduction(rest, rest);
		result = result + alpha * z;
		z = new_rest + beta * z;
		rest = new_rest;
	}
	result.print(std::cout);
    delete[] offsets; delete[] subSizes;

	MPI_Finalize();
	return 0;
}
