#include "linears.h"

#include <omp.h>

double square_norm(const double *vector, int size) {
	return dot_production(vector, vector, size);
}

void add_vector(const double *a, const double *b, double *sum, int size) {
	#pragma omp for
	for(int index = 0; index < size; index++)
		sum[index] = a[index] + b[index];
}

void sub_vector(const double *a, const double *b, double *sub, int size) {
	#pragma omp for
	for(int index = 0; index < size; index++)
		sub[index] = a[index] - b[index];
}

void mul_matrix_vector(const double *matrix, const double *vector, double *outVector, int size) {
	#pragma omp for
	for(int row = 0; row < size; row++) {
        	outVector[row] = 0;
		for(int column = 0; column < size; column++)
			outVector[row] += matrix[row * size + column] * vector[column];
	}
}


void mul_scalar_vector(double scalar, const double *in, double *out, int size) {
	#pragma omp for
	for(int index = 0; index < size; index++)
		out[index] = in[index] * scalar;
}

double dot_production(const double *a, const double *b, int size) {
	double res = 0;
	//TODO parallel
	for(int index = 0; index < size; index++)
		res += a[index] * b[index];
	return res;
}

std::ostream &print_vector(const double *vector, std::ostream &out, int size) {
	out << vector[0];
	for(int index = 1; index < size; index++)
		out << ' ' << vector[index];
	return out;
}
