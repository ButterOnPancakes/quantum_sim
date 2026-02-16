#include "matrices.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <complex.h>

typedef struct matrix {
    int rows;
    int cols;
    double complex **data;
} Matrix;

Matrix *matrix_zero(int rows, int cols) {
    assert(rows > 0 && cols > 0);

    Matrix *mat = malloc(sizeof(Matrix));
    mat->rows = rows;
    mat->cols = cols;
    mat->data = (double complex **)malloc(rows * sizeof(double complex *));

    //#pragma omp parallel for
    for (int i = 0; i < rows; i++) {
        mat->data[i] = (double complex *)calloc(cols, sizeof(double complex));
    }

    return mat;
}
Matrix *matrix_identity(int size) {
    Matrix *mat = matrix_zero(size, size);
    for (int i = 0; i < size; i++) {
        mat->data[i][i] = 1.0;
    }
    return mat;
}
Matrix *matrix_of_array(int rows, int cols, double complex data[rows][cols]) {
    Matrix *mat = malloc(sizeof(Matrix));
    mat->rows = rows;
    mat->cols = cols;
    mat->data = (double complex **)malloc(rows * sizeof(double complex *));

    //#pragma omp parallel for
    for (int i = 0; i < rows; i++) {
        mat->data[i] = (double complex *)calloc(cols, sizeof(double complex));
        for(int j = 0; j < cols; j++) {
            mat->data[i][j] = data[i][j];
        }
    }

    return mat;

}
void matrix_free(Matrix *mat) {
    for (int i = 0; i < mat->rows; i++) {
        free(mat->data[i]);
    }
    free(mat->data);
    free(mat);
}

double complex matrix_get(Matrix *m, int i, int j) {
    assert(0 <= i && i < m->rows && 0 <= j && j < m->cols);
    return m->data[i][j];
}
int matrix_nb_lines(Matrix *mat) {
    return mat->rows;
}
int matrix_nb_column(Matrix *mat) {
    return mat->cols;
}
void matrix_print(Matrix *mat) {
    for (int i = 0; i < mat->rows; i++) {
        for (int j = 0; j < mat->cols; j++) {
            //printf("(%.2f + i%.2f) ", creal(mat->data[i][j]), cimag(mat->data[i][j]));
            printf("%.2f ", creal(mat->data[i][j]));
        }
        printf("\n");
    }
}

void matrix_set(Matrix *mat, int i, int j, double complex c) {
    assert(0 <= i && i < mat->rows && 0 <= j && j < mat->cols);
    mat->data[i][j] = c;
}

Matrix *matrix_duplicate(Matrix *mat) {
    Matrix *res = matrix_zero(mat->rows, mat->cols);

    //#pragma omp parallel for
    for(int i = 0; i < mat->rows; i++) {
        for(int j = 0; j < mat->cols; j++) {
            res->data[i][j] = mat->data[i][j];
        }
    }
    return res;
}
void matrix_copy(Matrix *dest, Matrix *src) {
    assert(dest->cols == src->cols && dest->rows == src->rows);

    //#pragma omp parallel for
    for(int i = 0; i < dest->rows; i++) {
        for(int j = 0; j < dest->cols; j++) {
            dest->data[i][j] = src->data[i][j];
        }
    }
}

double matrix_norm(Matrix *A) {
    double sum = 0.0;
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            sum += cabs(A->data[i][j]) * cabs(A->data[i][j]);
        }
    }
    return sqrt(sum);
}
void matrix_normalise(Matrix *mat) {
    double norm = matrix_norm(mat);
    if(norm < 0.01) return;
    for(int i = 0; i < mat->rows; i++) {
        for(int j = 0; j < mat->cols; j++) {
            mat->data[i][j] = mat->data[i][j] / norm;
        }
    }
}

void matrix_add(Matrix *A, Matrix *B, Matrix **out) {
    if (A->rows != B->rows || A->cols != B->cols) {
        fprintf(stderr, "Error: Matrix dimensions must agree for addition.\n");
        exit(EXIT_FAILURE);
    }
    //Can do it in place
    //#pragma omp parallel for
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            (*out)->data[i][j] = A->data[i][j] + B->data[i][j];
        }
    }
}
void matrix_mult(Matrix *A, Matrix *B, Matrix **out) {
    if (A->cols != B->rows) {
        fprintf(stderr, "Error: Matrix dimensions must agree for multiplication.\n");
        exit(EXIT_FAILURE);
    }
    //Cant do in place
    Matrix *C = matrix_zero(A->rows, B->cols);

    //#pragma omp parallel for
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < B->cols; j++) {
            for (int k = 0; k < A->cols; k++) {
                C->data[i][j] += A->data[i][k] * B->data[k][j];
            }
        }
    }

    matrix_free(*out);
    *out = C;
}
void matrix_tensor_product(Matrix *A, Matrix *B, Matrix **out) {
    Matrix *C = matrix_zero(A->rows * B->rows, A->cols * B->cols);

    //#pragma omp parallel for
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            for (int k = 0; k < B->rows; k++) {
                for (int l = 0; l < B->cols; l++) {
                    C->data[i * B->rows + k][j * B->cols + l] = A->data[i][j] * B->data[k][l];
                }
            }
        }
    }
    
    matrix_free(*out);
    *out = C;
}
void matrix_tensor_product_stack(Matrix *A, int rowsB, int colsB, double complex dataB[rowsB][colsB], Matrix **out) {
    Matrix *C = matrix_zero(A->rows * rowsB, A->cols * colsB);

    //#pragma omp parallel for
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            for (int k = 0; k < rowsB; k++) {
                for (int l = 0; l < colsB; l++) {
                    C->data[i * rowsB + k][j * colsB + l] = A->data[i][j] * dataB[k][l];
                }
            }
        }
    }

    matrix_free(*out);
    *out = C;
}