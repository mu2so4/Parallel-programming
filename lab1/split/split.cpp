#include "linears.h"

double &Matrix::operator[](int index) const {
	return data[index];
}

Vector::Vector(const Vector &vector): size(vector.size), subSize(vector.subSize) {
    data = new double[subSize];
    for(int index = 0; index < subSize; index++)
		data[index] = vector.data[index];
}

Vector::Vector(Vector &&vector) noexcept: size(vector.size), subSize(vector.subSize) {
    data = vector.data;
    vector.data = nullptr;
}

Vector &Vector::operator=(const Vector &vector) {
	if(&vector != this) {
    	for(int index = 0; index < vector.subSize; index++)
		data[index] = vector.data[index];
	}
	return *this;
}

Vector &Vector::operator=(Vector &&vector) noexcept {
	if(&vector != this) {
        size = vector.size;
        subSize = vector.subSize;
        delete[] data;
    	data = vector.data;
        vector.data = nullptr;
	}
	return *this;
}

void Vector::print(std::ostream &out) const {
    double *res = nullptr;
    if(!rank)
        res = new double[size];
    
    MPI_Gatherv(data, subSize, MPI_DOUBLE, res, subSizes, offsets, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    endTime = MPI_Wtime();
    if(!rank) {
        out << res[0];
        for(int index = 1; index < size; index++)
	        out << ' ' << res[index];
        out << '\n';
    }
    delete[] res;
}

double Vector::squareNorm() const {
	return dotProduction(*this, *this);
}

double &Vector::operator[](int index) const {
	return data[index];
}

Vector operator+(Vector a, const Vector &b) {
    for(int index = 0; index < a.subSize; index++)   
        a[index] += b[index];
    return a;
}

Vector operator-(Vector a, const Vector &b) {
    for(int index = 0; index < a.subSize; index++)   
        a[index] -= b[index];
    return a;
}


Vector operator*(const Matrix &mat, Vector vec) {
	double *subRes = new double[vec.size], *res = new double[vec.size];
	for(int row = 0; row < vec.size; row++) {
        subRes[row] = 0;
		for(int column = 0; column < vec.subSize; column++)
			subRes[row] += mat[row * vec.subSize + column] * vec[column];
	}
    MPI_Allreduce(subRes, res, vec.size, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    for(int index = 0; index < vec.subSize; index++) {
        vec[index] = res[index + vec.size * rank / processCount];
    }
	delete[] res;
    delete[] subRes;
	return vec;
}


Vector operator*(double scalar, Vector vec) {
    for(int index = 0; index < vec.subSize; index++) {
        vec[index] *= scalar;
    }
	return vec;
}

double Vector::dotProduction(const Vector &a, const Vector &b) {
	double res = 0, subRes = 0;
	for(int index = 0; index < a.subSize; index++)
		subRes += a[index] * b[index];
	MPI_Allreduce(&subRes, &res, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	return res;
}

