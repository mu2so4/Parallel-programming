#include <iostream>
#include <fstream>
#include "series/linears.h"

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
	Vector b0(sle, size), x(ans, size);
	Matrix mat(sle, size);
	sle.close();
	ans.close();

	Vector b = mat * x;

	out << "Real:\n";
	b0.print(out);
	out << "\nEvaluated:\n";
	b.print(out);
	return 0;
}
