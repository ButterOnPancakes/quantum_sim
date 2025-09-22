#include "matrices.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>

#define TOLERANCE 1e-60

// Test utilities
int complex_compare(double complex a, double complex b) {
    return fabs(creal(a) - creal(b)) < TOLERANCE && 
           fabs(cimag(a) - cimag(b)) < TOLERANCE;
}

int matrix_compare(Matrix A, Matrix B) {
    if (A.rows != B.rows || A.cols != B.cols) return 0;
    
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < A.cols; j++) {
            if (!complex_compare(A.data[i][j], B.data[i][j])) {
                return 0;
            }
        }
    }
    return 1;
}

void print_test_result(const char* test_name, int passed) {
    printf("%s: %s\n", test_name, passed ? "PASS" : "FAIL");
}

// Test functions
void test_matrix_zero() {
    Matrix zero = matrix_zero(2, 3);
    double complex expected = 0.0 + 0.0*I;
    int passed = 1;
    
    for (int i = 0; i < zero.rows; i++) {
        for (int j = 0; j < zero.cols; j++) {
            if (!complex_compare(zero.data[i][j], expected)) {
                passed = 0;
                break;
            }
        }
    }
    
    print_test_result("Matrix Zero", passed);
    matrix_free(zero);
}

void test_matrix_identity() {
    Matrix id = matrix_identity(3);
    double complex zero = 0.0 + 0.0*I;
    double complex one = 1.0 + 0.0*I;
    int passed = 1;
    
    for (int i = 0; i < id.rows; i++) {
        for (int j = 0; j < id.cols; j++) {
            double complex expected = (i == j) ? one : zero;
            if (!complex_compare(id.data[i][j], expected)) {
                passed = 0;
                break;
            }
        }
    }
    
    print_test_result("Matrix Identity", passed);
    matrix_free(id);
}

void test_matrix_add() {
    Matrix A = matrix_zero(2, 2);
    Matrix B = matrix_zero(2, 2);
    
    // Fill matrices with test values
    A.data[0][0] = 1.0 + 2.0*I;
    A.data[0][1] = 3.0 + 4.0*I;
    A.data[1][0] = 5.0 + 6.0*I;
    A.data[1][1] = 7.0 + 8.0*I;
    
    B.data[0][0] = 2.0 + 1.0*I;
    B.data[0][1] = 4.0 + 3.0*I;
    B.data[1][0] = 6.0 + 5.0*I;
    B.data[1][1] = 8.0 + 7.0*I;
    
    Matrix result = matrix_add(A, B);
    
    // Expected result
    Matrix expected = matrix_zero(2, 2);
    expected.data[0][0] = 3.0 + 3.0*I;
    expected.data[0][1] = 7.0 + 7.0*I;
    expected.data[1][0] = 11.0 + 11.0*I;
    expected.data[1][1] = 15.0 + 15.0*I;
    
    int passed = matrix_compare(result, expected);
    print_test_result("Matrix Add", passed);
    
    matrix_free(A);
    matrix_free(B);
    matrix_free(result);
    matrix_free(expected);
}

void test_matrix_mult() {
    Matrix A = matrix_zero(2, 3);
    Matrix B = matrix_zero(3, 2);
    
    // Fill matrices with test values
    A.data[0][0] = 1.0 + 0.0*I;
    A.data[0][1] = 2.0 + 0.0*I;
    A.data[0][2] = 3.0 + 0.0*I;
    A.data[1][0] = 4.0 + 0.0*I;
    A.data[1][1] = 5.0 + 0.0*I;
    A.data[1][2] = 6.0 + 0.0*I;
    
    B.data[0][0] = 7.0 + 0.0*I;
    B.data[0][1] = 8.0 + 0.0*I;
    B.data[1][0] = 9.0 + 0.0*I;
    B.data[1][1] = 10.0 + 0.0*I;
    B.data[2][0] = 11.0 + 0.0*I;
    B.data[2][1] = 12.0 + 0.0*I;
    
    Matrix result = matrix_mult(A, B);
    
    // Expected result
    Matrix expected = matrix_zero(2, 2);
    expected.data[0][0] = 58.0 + 0.0*I;   // 1*7 + 2*9 + 3*11
    expected.data[0][1] = 64.0 + 0.0*I;   // 1*8 + 2*10 + 3*12
    expected.data[1][0] = 139.0 + 0.0*I;  // 4*7 + 5*9 + 6*11
    expected.data[1][1] = 154.0 + 0.0*I;  // 4*8 + 5*10 + 6*12
    
    int passed = matrix_compare(result, expected);
    print_test_result("Matrix Multiply", passed);
    
    matrix_free(A);
    matrix_free(B);
    matrix_free(result);
    matrix_free(expected);
}

void test_matrix_tensor_product() {
    Matrix A = matrix_zero(2, 2);
    Matrix B = matrix_zero(2, 2);
    
    // Fill matrices with test values
    A.data[0][0] = 1.0 + 0.0*I;
    A.data[0][1] = 2.0 + 0.0*I;
    A.data[1][0] = 3.0 + 0.0*I;
    A.data[1][1] = 4.0 + 0.0*I;
    
    B.data[0][0] = 5.0 + 0.0*I;
    B.data[0][1] = 6.0 + 0.0*I;
    B.data[1][0] = 7.0 + 0.0*I;
    B.data[1][1] = 8.0 + 0.0*I;
    
    Matrix result = matrix_tensor_product(A, B);
    
    // Expected result (4x4 matrix)
    Matrix expected = matrix_zero(4, 4);
    expected.data[0][0] = 5.0 + 0.0*I;   // 1*5
    expected.data[0][1] = 6.0 + 0.0*I;   // 1*6
    expected.data[0][2] = 10.0 + 0.0*I;  // 2*5
    expected.data[0][3] = 12.0 + 0.0*I;  // 2*6
    
    expected.data[1][0] = 7.0 + 0.0*I;   // 1*7
    expected.data[1][1] = 8.0 + 0.0*I;   // 1*8
    expected.data[1][2] = 14.0 + 0.0*I;  // 2*7
    expected.data[1][3] = 16.0 + 0.0*I;  // 2*8
    
    expected.data[2][0] = 15.0 + 0.0*I;  // 3*5
    expected.data[2][1] = 18.0 + 0.0*I;  // 3*6
    expected.data[2][2] = 20.0 + 0.0*I;  // 4*5
    expected.data[2][3] = 24.0 + 0.0*I;  // 4*6
    
    expected.data[3][0] = 21.0 + 0.0*I;  // 3*7
    expected.data[3][1] = 24.0 + 0.0*I;  // 3*8
    expected.data[3][2] = 28.0 + 0.0*I;  // 4*7
    expected.data[3][3] = 32.0 + 0.0*I;  // 4*8
    
    int passed = matrix_compare(result, expected);
    print_test_result("Matrix Tensor Product", passed);
    
    matrix_free(A);
    matrix_free(B);
    matrix_free(result);
    matrix_free(expected);
}

void test_matrix_add_override() {
    Matrix A = matrix_zero(2, 2);
    Matrix B = matrix_zero(2, 2);
    
    // Fill matrices with test values
    A.data[0][0] = 1.0 + 2.0*I;
    A.data[0][1] = 3.0 + 4.0*I;
    A.data[1][0] = 5.0 + 6.0*I;
    A.data[1][1] = 7.0 + 8.0*I;
    
    B.data[0][0] = 2.0 + 1.0*I;
    B.data[0][1] = 4.0 + 3.0*I;
    B.data[1][0] = 6.0 + 5.0*I;
    B.data[1][1] = 8.0 + 7.0*I;
    
    // Make a copy of A for comparison
    Matrix A_copy = matrix_zero(2, 2);
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < A.cols; j++) {
            A_copy.data[i][j] = A.data[i][j];
        }
    }
    
    matrix_add_override(&A, &B);
    
    // Expected result
    Matrix expected = matrix_zero(2, 2);
    expected.data[0][0] = 3.0 + 3.0*I;
    expected.data[0][1] = 7.0 + 7.0*I;
    expected.data[1][0] = 11.0 + 11.0*I;
    expected.data[1][1] = 15.0 + 15.0*I;
    
    int passed = matrix_compare(A, expected);
    print_test_result("Matrix Add Override", passed);
    
    matrix_free(A);
    matrix_free(B);
    matrix_free(A_copy);
    matrix_free(expected);
}

void test_matrix_mult_override() {
    Matrix A = matrix_zero(2, 3);
    Matrix B = matrix_zero(3, 2);
    
    // Fill matrices with test values
    A.data[0][0] = 1.0 + 0.0*I;
    A.data[0][1] = 2.0 + 0.0*I;
    A.data[0][2] = 3.0 + 0.0*I;
    A.data[1][0] = 4.0 + 0.0*I;
    A.data[1][1] = 5.0 + 0.0*I;
    A.data[1][2] = 6.0 + 0.0*I;
    
    B.data[0][0] = 7.0 + 0.0*I;
    B.data[0][1] = 8.0 + 0.0*I;
    B.data[1][0] = 9.0 + 0.0*I;
    B.data[1][1] = 10.0 + 0.0*I;
    B.data[2][0] = 11.0 + 0.0*I;
    B.data[2][1] = 12.0 + 0.0*I;
    
    // Make a copy of A for comparison
    Matrix A_copy = matrix_zero(2, 3);
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < A.cols; j++) {
            A_copy.data[i][j] = A.data[i][j];
        }
    }
    
    matrix_mult_override(&A, &B);
    
    // Expected result (2x2 matrix)
    Matrix expected = matrix_zero(2, 2);
    expected.data[0][0] = 58.0 + 0.0*I;   // 1*7 + 2*9 + 3*11
    expected.data[0][1] = 64.0 + 0.0*I;   // 1*8 + 2*10 + 3*12
    expected.data[1][0] = 139.0 + 0.0*I;  // 4*7 + 5*9 + 6*11
    expected.data[1][1] = 154.0 + 0.0*I;  // 4*8 + 5*10 + 6*12
    
    int passed = matrix_compare(A, expected);
    print_test_result("Matrix Multiply Override", passed);
    
    matrix_free(A);
    matrix_free(B);
    matrix_free(A_copy);
    matrix_free(expected);
}

void test_matrix_tensor_product_override() {
    Matrix A = matrix_zero(2, 2);
    Matrix B = matrix_zero(2, 2);
    
    // Fill matrices with test values
    A.data[0][0] = 1.0 + 0.0*I;
    A.data[0][1] = 2.0 + 0.0*I;
    A.data[1][0] = 3.0 + 0.0*I;
    A.data[1][1] = 4.0 + 0.0*I;
    
    B.data[0][0] = 5.0 + 0.0*I;
    B.data[0][1] = 6.0 + 0.0*I;
    B.data[1][0] = 7.0 + 0.0*I;
    B.data[1][1] = 8.0 + 0.0*I;
    
    // Make a copy of A for comparison
    Matrix A_copy = matrix_zero(2, 2);
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < A.cols; j++) {
            A_copy.data[i][j] = A.data[i][j];
        }
    }
    
    matrix_tensor_product_override(&A, &B);
    
    // Expected result (4x4 matrix)
    Matrix expected = matrix_zero(4, 4);
    expected.data[0][0] = 5.0 + 0.0*I;   // 1*5
    expected.data[0][1] = 6.0 + 0.0*I;   // 1*6
    expected.data[0][2] = 10.0 + 0.0*I;  // 2*5
    expected.data[0][3] = 12.0 + 0.0*I;  // 2*6
    
    expected.data[1][0] = 7.0 + 0.0*I;   // 1*7
    expected.data[1][1] = 8.0 + 0.0*I;   // 1*8
    expected.data[1][2] = 14.0 + 0.0*I;  // 2*7
    expected.data[1][3] = 16.0 + 0.0*I;  // 2*8
    
    expected.data[2][0] = 15.0 + 0.0*I;  // 3*5
    expected.data[2][1] = 18.0 + 0.0*I;  // 3*6
    expected.data[2][2] = 20.0 + 0.0*I;  // 4*5
    expected.data[2][3] = 24.0 + 0.0*I;  // 4*6
    
    expected.data[3][0] = 21.0 + 0.0*I;  // 3*7
    expected.data[3][1] = 24.0 + 0.0*I;  // 3*8
    expected.data[3][2] = 28.0 + 0.0*I;  // 4*7
    expected.data[3][3] = 32.0 + 0.0*I;  // 4*8
    
    int passed = matrix_compare(A, expected);
    print_test_result("Matrix Tensor Product Override", passed);
    
    matrix_free(A);
    matrix_free(B);
    matrix_free(A_copy);
    matrix_free(expected);
}

int main() {
    printf("Testing Matrix Library with double complex Numbers:\n");
    printf("============================================\n");
    
    test_matrix_zero();
    test_matrix_identity();
    test_matrix_add();
    test_matrix_mult();
    test_matrix_tensor_product();
    test_matrix_add_override();
    test_matrix_mult_override();
    test_matrix_tensor_product_override();
    
    printf("============================================\n");
    
    return 0;
}