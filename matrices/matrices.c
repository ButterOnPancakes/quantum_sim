#include "matrices.h"

#include <stdlib.h>
#include <stdio.h>
#include "../complex/complexes.h"

struct mat {
    int rows;
    int cols;
    Complex **data;
};

Matrix matrix_zero(int rows, int cols) {
    Matrix mat;
    mat.rows = rows;
    mat.cols = cols;
    mat.data = (Complex **)malloc(rows * sizeof(Complex *));
    for (int i = 0; i < rows; i++) {
        mat.data[i] = (Complex *)calloc(cols, sizeof(Complex));
    }
    return mat;
}
Matrix matrix_identity(int size) {
    Matrix mat = matrix_zero(size, size);
    for (int i = 0; i < size; i++) {
        mat.data[i][i] = complex_create(1.0, 0.0);
    }
    return mat;
}
void matrix_free(Matrix mat) {
    for (int i = 0; i < mat.rows; i++) {
        free(mat.data[i]);
    }
    free(mat.data);
}

void matrix_print(Matrix mat) {
    for (int i = 0; i < mat.rows; i++) {
        for (int j = 0; j < mat.cols; j++) {
            printf("(%lf + i%lf) ", mat.data[i][j].real, mat.data[i][j].imag);
        }
        printf("\n");
    }
}

Matrix matrix_add(Matrix A, Matrix B) {
    if (A.rows != B.rows || A.cols != B.cols) {
        fprintf(stderr, "Error: Matrix dimensions must agree for addition.\n");
        exit(EXIT_FAILURE);
    }
    Matrix C = matrix_zero(A.rows, A.cols);
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < A.cols; j++) {
            C.data[i][j] = complex_add(A.data[i][j], B.data[i][j]);
        }
    }
    return C;
}
Matrix matrix_mult(Matrix A, Matrix B) {
    if (A.cols != B.rows) {
        fprintf(stderr, "Error: Matrix dimensions must agree for multiplication.\n");
        exit(EXIT_FAILURE);
    }
    Matrix C = matrix_zero(A.rows, B.cols);
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < B.cols; j++) {
            for (int k = 0; k < A.cols; k++) {
                C.data[i][j] = complex_add(C.data[i][j], complex_mult(A.data[i][k], B.data[k][j]));
            }
        }
    }
    return C;
}
Matrix matrix_tensor_product(Matrix A, Matrix B) {
    Matrix C = matrix_zero(A.rows * B.rows, A.cols * B.cols);
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < A.cols; j++) {
            for (int k = 0; k < B.rows; k++) {
                for (int l = 0; l < B.cols; l++) {
                    C.data[i * B.rows + k][j * B.cols + l] = complex_mult(A.data[i][j], B.data[k][l]);
                }
            }
        }
    }
    return C;
}

void matrix_add_override(Matrix *A, Matrix *B) {
    if (A->rows != B->rows || A->cols != B->cols) {
        fprintf(stderr, "Error: Matrix dimensions must agree for addition.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            A->data[i][j] = complex_add(A->data[i][j], B->data[i][j]);
        }
    }
}
void matrix_mult_override(Matrix *A, Matrix *B) {
    if (A->cols != B->rows) {
        fprintf(stderr, "Error: Matrix dimensions must agree for multiplication.\n");
        exit(EXIT_FAILURE);
    }
    Matrix C = matrix_mult(*A, *B);
    matrix_free(*A);
    *A = C;
}
void matrix_tensor_product_override(Matrix *A, Matrix *B) {
    Matrix C = matrix_tensor_product(*A, *B);
    matrix_free(*A);
    *A = C;
}