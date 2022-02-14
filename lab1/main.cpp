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
    delete[] arrx0;

    double alpha;
    double beta;
	std::ofstream eps("e.txt");

    while(rest.norma() / b.norma() >= epsilon) {
		eps << rest.norma() / b.norma() << '\n'; 
		Vector z_1 = koefs * z;
        alpha = Vector::dotProduction(rest, rest) / Vector::dotProduction(z_1, z);
		Vector new_rest = rest - alpha * z_1;
		beta = Vector::dotProduction(new_rest, new_rest) / Vector::dotProduction(rest, rest);
		result = result + alpha * z;
		z = new_rest + beta * z;
		rest = new_rest;
    }

	result.print(std::cout);
	eps.close();
    return 0;
}
