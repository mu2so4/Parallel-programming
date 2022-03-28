#include <iostream>
#include <fstream>
#include "linears.h"

int main(int argc, char **argv) {
	if(argc != 3) {
		std::cout << "Wrong input\n";
		return 1;
	}

	std::ifstream sle(argv[1]);
	std::ifstream ans(argv[2]);
	std::ofstream out("check.txt");
	
	int size;
	sle >> size;
	double *b0 = new double[size];
	double *x = new double[size];
	double *matrix = new double[size * size];
	double *b = new double[size];
	for(int index = 0; index < size; index++)
		sle >> b0[index];
	for(int index = 0; index < size * size; index++)
		sle >> matrix[index];
	sle.close();
	for(int index = 0; index < size; index++)
		ans >> x[index];
	ans.close();

	mul_matrix_vector(matrix, x, b, size);

	out << "Real:\n";
	print_vector(b0, out, size);
	out << "\nEvaluated:\n";
	print_vector(b, out, size) << '\n';

	delete[] b0;
	delete[] b;
	delete[] x;
	delete[] matrix;
	return 0;
}
