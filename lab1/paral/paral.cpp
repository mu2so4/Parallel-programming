#include "linears.h"

Matrix::Matrix(const double *mat, int dim): size(dim) {
    data = new double[size * size];
	for(int index = 0; index < size * size; index++)
		data[index] = mat[index];
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
    double *res = nullptr;
    if(!rank)
        res = new double[size];
    
    MPI_Gatherv(data + workZoneLeft, workZoneRight - workZoneLeft, MPI_DOUBLE, res, subSizes, offsets, MPI_DOUBLE, 0, MPI_COMM_WORLD);
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
    //double *buf = new double[workZoneRight - workZoneLeft];
    for(int index = workZoneLeft; index < workZoneRight; index++)   
        a[index] += b[index];
		//buf[index - workZoneLeft] = a[index] + b[index];

    //MPI_Allgatherv(buf, workZoneRight - workZoneLeft, MPI_DOUBLE, &a[0], subSizes, offsets, MPI_DOUBLE, MPI_COMM_WORLD);
    //delete[] buf;
    return a;
}

Vector operator-(Vector a, const Vector &b) {
    //double *buf = new double[workZoneRight - workZoneLeft];
    for(int index = workZoneLeft; index < workZoneRight; index++)   
        a[index] -= b[index];
		//buf[index - workZoneLeft] = a[index] - b[index];

    //MPI_Allgatherv(buf, workZoneRight - workZoneLeft, MPI_DOUBLE, &a[0], subSizes, offsets, MPI_DOUBLE, MPI_COMM_WORLD);
    //delete[] buf;
    return a;
}


Vector operator*(const Matrix &mat, Vector vec) {
	double *res = new double[vec.size];
	for(int row = 0; row < vec.size; row++) {
        res[row] = 0;
		for(int column = workZoneLeft; column < workZoneRight; column++)
			res[row] += mat[row * vec.size + column] * vec[column];
	}
    MPI_Allreduce(res, vec.data, vec.size, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	delete[] res;
	return vec;
}


Vector operator*(double scalar, Vector vec) {
    for(int index = workZoneLeft; index < workZoneRight; index++) {
        vec[index] *= scalar;
    }
	return vec;
}

double Vector::dotProduction(const Vector &a, const Vector &b) {
	double res = 0, subRes = 0;
	for(int index = workZoneLeft; index < workZoneRight; index++)
		subRes += a[index] * b[index];
    MPI_Allreduce(&subRes, &res, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	return res;
}

