#include "../simulator/matrices.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "../utils/utils.h"

Matrix *get_tensored_gate_mat(Matrix *mat, int i, int n) {
    Matrix *gateMat;
    int k = reverse_power(matrix_nb_column(mat));
    int end = i + k;

    if(i > 0) {
        gateMat = matrix_identity(1 << i);
        matrix_tensor_product(gateMat, mat, &gateMat);
    }
    else gateMat = matrix_duplicate(mat);


    if(end < n) {
        Matrix *complete_identity = matrix_identity(1 << (n - end));

        matrix_tensor_product(gateMat, complete_identity, &gateMat);
        matrix_free(complete_identity);
    }

    return gateMat;
}

Matrix *proj0_tensored(int k, int n) {
    uint64_t size = 1 << n;
    uint64_t add = 1 << (n - k);

    Matrix *mat = matrix_zero(size, size);
    for(uint64_t i = 0; i < size; i += add) {
        for(uint64_t j = i; j < i + add / 2; j++) {
            matrix_set(mat, j, j, 1.);
        }
    }
    return mat;
}
Matrix *proj1_tensored(int k, int n) {
    uint64_t size = 1 << n;
    uint64_t add = 1 << (n - k);

    Matrix *mat = matrix_zero(size, size);
    for(uint64_t i = 0; i < size; i += add) {
        for(uint64_t j = i + add / 2; j < i + add; j++) {
            matrix_set(mat, j, j, 1.);
        }
    }
    return mat;
}

int main() {
    Matrix *proj = matrix_of_array(2, 2, (double complex[2][2]){
        {0, 0},
        {0, 1}
    });
    int n = 5;
    for(int i = 0; i < n; i++) {
        Matrix *measure = get_tensored_gate_mat(proj, i, n);
        matrix_print(measure);
        matrix_free(measure);
        Matrix *measure2 = proj1_tensored(i, n);
        matrix_print(measure2);
        matrix_free(measure2);
        printf("\n");
    }
    return EXIT_SUCCESS;
}