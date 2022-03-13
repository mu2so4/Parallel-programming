#include "linears.h"

Matrix::Matrix(int size) {
	data = new double[size * size];
}

Matrix::Matrix(std::istream &in, int length): size(length) {
	data = new double[size * size];
	for(int index = 0; index < size * size; index++) {
		in >> data[index];
		if(in.eof()) {
			std::cerr << "reached the end of file too early\n";
			throw std::exception();
		}
	}
}

double &Matrix::operator[](int index) const {
	return data[index];
}

Vector::Vector(int length): size(length) {
	data = new double[size];
}

Vector::Vector(const double *vec, int length): size(length) {
	data = new double[size];
	for(int index = 0; index < size; index++)
		data[index] = vec[index];
}

Vector::Vector(std::istream &in, int length): size(length) {
	data = new double[size];
	for(int index = 0; index < size; index++) {
		in >> data[index];
		if(in.eof()) {
			std::cerr << "reached the end of file too early\n";
			throw std::exception();
		}
	}
}

Vector::Vector(const Vector &vector): size(vector.size) {
    data = new double[size];
    for(int index = 0; index < size; index++)
		data[index] = vector.data[index];
}

Vector::Vector(Vector &&vector) noexcept: size(vector.size) {
    data = vector.data;
    vector.data = nullptr;
}

Vector &Vector::operator=(const Vector &vector) {
	if(&vector != this) {
    	for(int index = 0; index < vector.size; index++)
		data[index] = vector.data[index];
	}
	return *this;
}

Vector &Vector::operator=(Vector &&vector) noexcept {
	if(&vector != this) {
        size = vector.size;
        delete[] data;
    	data = vector.data;
        vector.data = nullptr;
	}
	return *this;
}

void Vector::print(std::ostream &out) const {
	out << data[0];
	for(int index = 1; index < size; index++)
		out << ' ' << data[index];
	out << '\n';
}

double Vector::squareNorm() const {
	return dotProduction(*this, *this);
}

double &Vector::operator[](int index) const {
	return data[index];
}

Vector operator+(Vector a, const Vector &b) {
    for(int index = 0; index < a.size; index++)
		a[index] += b[index];
    return a;
}

Vector operator-(Vector a, const Vector &b) {
    for(int index = 0; index < a.size; index++)
		a[index] -= b[index];
    return a;
}


Vector operator*(const Matrix &mat, Vector vec) {
	double *res = new double[vec.size];
	for(int row = 0; row < vec.size; row++) {
		for(int column = 0; column < vec.size; column++)
			res[row] += mat[row * vec.size + column] * vec[column];
	}
	delete[] vec.data;
	vec.data = res;
	return vec;
}


Vector operator*(double scalar, Vector vec) {
	for(int index = 0; index < vec.size; index++)
		vec[index] *= scalar;
	return vec;
}

double Vector::dotProduction(const Vector &a, const Vector &b) {
	double res = 0;
	for(int index = 0; index < a.size; index++)
		res += a[index] * b[index];
	return res;
}

