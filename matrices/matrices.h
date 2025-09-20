#ifndef MATRICES_H
#define MATRICES_H

typedef struct mat Matrix;

Matrix matrix_zero(int rows, int cols);
Matrix matrix_identity(int size);
void matrix_free(Matrix mat);

void matrix_print(Matrix mat);

Matrix matrix_add(Matrix A, Matrix B);
Matrix matrix_mult(Matrix A, Matrix B);
Matrix matrix_tensor_product(Matrix A, Matrix B);

void matrix_add_override(Matrix *A, Matrix *B);
void matrix_mult_override(Matrix *A, Matrix *B);
void matrix_tensor_product_override(Matrix *A, Matrix *B);

#endif