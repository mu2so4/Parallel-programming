#include <fstream>
#include <cstdlib>
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
	Vector b(in, size);
	Matrix koefs(in, size);
	in.close();


	double *arrx0 = new double[size];
	for(int index = 0; index < size; index++)
		arrx0[index] = 1;
	Vector result(arrx0, size), rest = b - koefs * result, z = rest;
    //z.print(std::cerr);

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
	return 0;
}
