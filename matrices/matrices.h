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
Matrix matrix_of_array(int rows, int cols, double complex data[rows][cols]);
void matrix_free(Matrix mat);

double complex matrix_get(Matrix mat, int i, int j);
int matrix_nb_lines(Matrix mat);
int matrix_nb_column(Matrix mat);

void matrix_print(Matrix mat);

void matrix_set(Matrix mat, int i, int j, double complex c);

Matrix matrix_duplicate(Matrix A);
void matrix_copy(Matrix *dest, Matrix *src);

double matrix_norm(Matrix A);
void matrix_normalise(Matrix mat);

void matrix_add(Matrix *A, Matrix *B, Matrix *out);
//void matrix_add_stack(Matrix *A, int rowsB, int colsB, double complex dataB[rowsB][colsB], Matrix *out);

void matrix_mult(Matrix *A, Matrix *B, Matrix *out);
//void matrix_mult_stack(Matrix *A, int rowsB, int colsB, double complex dataB[rowsB][colsB], Matrix *out);

void matrix_tensor_product(Matrix *A, Matrix *B, Matrix *out);
void matrix_tensor_product_stack(Matrix *A, int rowsB, int colsB, double complex dataB[rowsB][colsB], Matrix *out);

#endif