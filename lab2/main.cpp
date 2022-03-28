#include <fstream>
#include <iostream>
#include "linears.h"

int main(int argc, char** argv) {
	if(argc != 3) {
		std::cout << "Wrong input\n";
		return 1;
	}

	double epsilon = atof(argv[1]);
	std::ifstream in(argv[2]);
	int size;
	in >> size;
	double *b = new double[size];
	double *koefs = new double[size * size];
	for(int index = 0; index < size; index++)
		in >> b[index];
	for(int index = 0; index < size * size; index++)
		in >> koefs[index];
	in.close();


	double *result = new double[size];
	double *buffer = new double[size];
	double *rest = new double[size];
	double *new_rest = new double[size];
       	double *z = new double[size];
	double *z_1 = new double[size];

	for(int index = 0; index < size; index++)
		result[index] = 1;
	mul_matrix_vector(koefs, result, buffer, size);
	sub_vector(b, buffer, rest, size);
	for(int index = 0; index < size; index++)
		z[index] = rest[index];

	double control = square_norm(b, size) * epsilon * epsilon, r2 = square_norm(rest, size);
	for(int iter = 0; iter < 10000 && r2 >= control; iter++) {
		mul_matrix_vector(koefs, z, z_1, size);
		double alpha = r2 / dot_production(z_1, z, size);

		mul_scalar_vector(alpha, z_1, buffer, size);
		sub_vector(rest, buffer, new_rest, size);

		double nr2 = square_norm(new_rest, size), beta = nr2 / r2;
		mul_scalar_vector(alpha, z, buffer, size);
		add_vector(result, buffer, result, size);

		mul_scalar_vector(beta, z, buffer, size);
		add_vector(new_rest, buffer, z, size);
		std::swap(rest, new_rest);
        	r2 = nr2;
	}

	std::cout << result[0];
	for(int index = 1; index < size; index++)
		std::cout << ' ' << result[index];
	std::cout << '\n';

	delete[] b;
	delete[] koefs;
	delete[] result;
	delete[] buffer;
	delete[] rest;
	delete[] new_rest;
	delete[] z;
	delete[] z_1;
	return 0;
}
