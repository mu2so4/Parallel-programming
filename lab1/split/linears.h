#ifndef LINEARS_H
#define LINEARS_H

#include <iostream>
#include <cmath>
#include <mpi.h>

extern int rank;
extern int processCount;
extern int *offsets;
extern int *subSizes;

class Vector;

class Matrix {
    int size;
    int subSize;
    double *data;

public:
    Matrix(const double *mat, int size, int subSize);

    ~Matrix() { delete[] data; }

    double &operator[](int index) const;
};

class Vector {
    int size;
    int subSize;
    double *data;

public:
    explicit Vector(int size, int subSize);
    Vector(const double *vec, int length, int subSize);
    Vector(const Vector &v);
    Vector(Vector &&v) noexcept;

    ~Vector() { delete[] data; }

    Vector &operator=(const Vector &vector);
    Vector &operator=(Vector &&vector) noexcept;

    void print(std::ostream &out) const;

    double squareNorm() const;

    double &operator[](int index) const;
    friend Vector operator+(Vector a, const Vector &b);
    friend Vector operator-(Vector a, const Vector &b);
    friend Vector operator*(const Matrix &mat, Vector vec);
    friend Vector operator*(double scalar, Vector vec);

    static double dotProduction(const Vector &a, const Vector &b);
};


#endif //LINEARS_H
