#include <iostream>
#include <climits>

double random_double() {
	return rand() / (double) INT_MAX * 200 - 100;
}

std::ostream &print_matrix(const double *matrix, int rows, int columns, std::ostream &out) {
	for(int x = 0; x < rows; x++) {
		out << matrix[x * columns];
		for(int y = 1; y < columns; y++)
			out << '\t' << matrix[x * columns + y];
		out << '\n';
	}
	return out;
}

int main(int argc, char **argv) {
	if(argc != 4) {
		std::cerr << "Got " << argc << " argument(s) instead of 3\n";
		return 1;
	}
	int n = atoi(argv[1]);
	int k = atoi(argv[2]);
	int m = atoi(argv[3]);
	double *A = new double[n * k];
	double *B = new double[k * m];
	double *C = new double[n * m];
	srand(time(0));

	for(int index = 0; index < n * k; index++)
		A[index] = random_double();
	for(int index = 0; index < k * m; index++)
		B[index] = random_double();

	print_matrix(A, n, k, std::cout) << '\n';
	print_matrix(B, k, m, std::cout) << '\n';

	for(int row = 0; row < n; row++) {
		for(int column = 0; column < m; column++) {
			C[row * m + column] = 0;
			for(int index = 0; index < k; index++) {
				C[row * m + column] += A[row * m + index] * B[index * m + column];
			}
		}
	}

	print_matrix(C, n, m, std::cout);

	delete[] A;
	delete[] B;
	delete[] C;
	return 0;
}
