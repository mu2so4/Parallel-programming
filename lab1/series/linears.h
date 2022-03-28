#ifndef LINEARS_H
#define LINEARS_H

#include <iostream>

class Vector;

class Matrix {
    int size;
    double *data;

public:
    explicit Matrix(std::istream &in, int size);

    ~Matrix() { delete[] data; }

    double &operator[](int index) const;
};

class Vector {
    int size;
    double *data;

public:
    explicit Vector(int size);
    Vector(double *vec, int length): size(length), data(vec) {}
    Vector(std::istream &in, int size);
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
