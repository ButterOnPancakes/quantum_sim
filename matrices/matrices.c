#include "matrices.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <complex.h>

Matrix matrix_zero(int rows, int cols) {
    Matrix mat;
    mat.rows = rows;
    mat.cols = cols;
    mat.data = (double complex **)malloc(rows * sizeof(double complex *));
    for (int i = 0; i < rows; i++) {
        mat.data[i] = (double complex *)calloc(cols, sizeof(double complex));
    }
    return mat;
}
Matrix matrix_identity(int size) {
    Matrix mat = matrix_zero(size, size);
    for (int i = 0; i < size; i++) {
        mat.data[i][i] = 1.0;
    }
    return mat;
}
Matrix matrix_of_array(double complex **data, int rows, int cols) {
    Matrix mat;
    mat.rows = rows;
    mat.cols = cols;
    mat.data = (double complex **)malloc(rows * sizeof(double complex *));
    for (int i = 0; i < rows; i++) {
        mat.data[i] = (double complex *)calloc(cols, sizeof(double complex));
        for(int j = 0; j < cols; j++) {
            mat.data[i][j] = data[i][j];
        }
    }
    return mat;

}
void matrix_free(Matrix mat) {
    for (int i = 0; i < mat.rows; i++) {
        free(mat.data[i]);
    }
    free(mat.data);
}

double complex matrix_get(Matrix m, int i, int j) {
    assert(0 <= i && i < m.rows && 0 <= j && j < m.cols);
    return m.data[i][j];
}
int matrix_nb_lines(Matrix mat) {
    return mat.rows;
}
int matrix_nb_column(Matrix mat) {
    return mat.cols;
}
void matrix_print(Matrix mat) {
    for (int i = 0; i < mat.rows; i++) {
        for (int j = 0; j < mat.cols; j++) {
            printf("(%.2f + i%.2f) ", creal(mat.data[i][j]), cimag(mat.data[i][j]));
        }
        printf("\n");
    }
}

void matrix_set(Matrix mat, int i, int j, double complex c) {
    assert(0 <= i && i < mat.rows && 0 <= j && j < mat.cols);
    mat.data[i][j] = c;
}

Matrix matrix_add(Matrix A, Matrix B) {
    if (A.rows != B.rows || A.cols != B.cols) {
        fprintf(stderr, "Error: Matrix dimensions must agree for addition.\n");
        exit(EXIT_FAILURE);
    }
    Matrix C = matrix_zero(A.rows, A.cols);
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < A.cols; j++) {
            C.data[i][j] = A.data[i][j] + B.data[i][j];
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
                C.data[i][j] += A.data[i][k] * B.data[k][j];
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
                    C.data[i * B.rows + k][j * B.cols + l] = A.data[i][j] * B.data[k][l];
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
            A->data[i][j] += B->data[i][j];
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