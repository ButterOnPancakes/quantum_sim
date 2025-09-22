#ifndef MATRICES_H
#define MATRICES_H

#include <complex.h>

typedef struct {
    int rows;
    int cols;
    double complex **data;
} Matrix;

Matrix matrix_zero(int rows, int cols);
Matrix matrix_identity(int size);
Matrix matrix_of_array(double complex **data, int rows, int cols);
void matrix_free(Matrix mat);

double complex matrix_get(Matrix mat, int i, int j);
int matrix_nb_lines(Matrix mat);
int matrix_nb_column(Matrix mat);

void matrix_print(Matrix mat);

void matrix_set(Matrix mat, int i, int j, double complex c);

Matrix matrix_add(Matrix A, Matrix B);
Matrix matrix_mult(Matrix A, Matrix B);
Matrix matrix_tensor_product(Matrix A, Matrix B);

/* Put result in matrix A */
void matrix_add_override(Matrix *A, Matrix *B);
void matrix_mult_override(Matrix *A, Matrix *B);
void matrix_tensor_product_override(Matrix *A, Matrix *B);

#endif