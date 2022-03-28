#ifndef LINEARS_H
#define LINEARS_H

#include <ostream>

void mul_scalar_vector(double scalar, const double *in, double *out, int size);

void mul_matrix_vector(const double *matrix, const double *vector, double *outVector, int size);

void add_vector(const double *a, const double *b, double *sum, int size);

void sub_vector(const double *a, const double *b, double *sub, int size);

double dot_production(const double *a, const double *b, int size);

double square_norm(const double *vector, int size);

std::ostream &print_vector(const double *vector, std::ostream &out, int size);

#endif //LINEARS_H
