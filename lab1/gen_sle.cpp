#include <fstream>
#include <iostream>
#include <climits>

double random_double() {
	return rand() / (INT_MAX / 200.) - 100;
}

int main(int argc, char ** argv) {
	if(argc != 3) {
		std::cerr << "Wrong input\n";
		return 1;
	}

	int size = std::stoi(argv[1]);
	if(size <= 0) {
		std::cerr << "Size cannot be less than 1\n";
		return 1;
	}
	std::ofstream out(argv[2]);
	srand(time(0));
	
	out << size << '\n' << random_double();
	for(int index = 1; index < size; index++) {
		out << '\t' << random_double();
	}
	out << '\n';
	out.flush();

	double *arr = new double[size * size], *pre = new double[size * size];
	for(int row = 0; row < size; row++) {
		for(int column = row + 1; column < size; column++) {
			arr[row * size + column] = arr[column * size + row] = random_double();
			arr[(size + 1) * row] -= arr[row * size + column];
			arr[(size + 1) * column] -= arr[row * size + column];
		}
	}
	for(int index = 0; index < size; index++)
		arr[(size + 1) * index] += size + random_double() + 100;

	/*for(int row = 0; row < size; row++)
		for(int column = row; column < size; column++) {
			for(int index = 0; index < size; index++)
				arr[row * size + column] += pre[row * size + index] * pre[column * size + index];
			arr[column * size + row] = arr[row * size + column];
		}*/

	/*for(int index = 0; index < size; index++)
		arr[index * (size + 1)] += 200;

	for(int row = 0; row < size; row++)
		for(int column = row + 1; column < size; column++) {
			double semi = (arr[row * size + column] + arr[column * size + row]) / 2;
			arr[row * size + column] = arr[column * size + row] = semi; 
		}*/
	for(int row = 0; row < size; row++) {
		for(int column = 0; column < size; column++) {
			out << arr[row * size + column] << '\t';
		}
		out << '\n';
		out.flush();
	}

	delete[] arr;
	delete[] pre;
	return 0;
}
