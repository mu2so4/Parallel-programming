#ifndef LINEARS_H
#define LINEARS_H

#include <mpi.h>
#include <iostream>

extern int rank;
extern int processCount;
extern int workZoneLeft;
extern int subSize;
extern int *offsets;
extern int *subSizes;
extern double endTime;

class Vector;

class Matrix {
    int size;
    int subSize;
    double *data;

public:
    Matrix(double *mat, int rows, int columns): size(columns), subSize(rows), data(mat) {}

    ~Matrix() { delete[] data; }

    double &operator[](int index) const;
};

class Vector {
    int size;
    double *data;

public:
    Vector(double *vec, int length): size(length), data(vec) {}
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
