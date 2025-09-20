#include "matrices.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "../complex/complexes.h"

typedef struct mat {
    int rows;
    int cols;
    Complex **data;
} Matrix;

#define TOLERANCE 1e-15

// Test utilities
int complex_compare(Complex a, Complex b) {
    return fabs(a.real - b.real) < TOLERANCE && 
           fabs(a.imag - b.imag) < TOLERANCE;
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
    Complex expected = {0.0, 0.0};
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
    Complex zero = {0.0, 0.0};
    Complex one = {1.0, 0.0};
    int passed = 1;
    
    for (int i = 0; i < id.rows; i++) {
        for (int j = 0; j < id.cols; j++) {
            Complex expected = (i == j) ? one : zero;
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
    A.data[0][0] = complex_create(1.0, 2.0);
    A.data[0][1] = complex_create(3.0, 4.0);
    A.data[1][0] = complex_create(5.0, 6.0);
    A.data[1][1] = complex_create(7.0, 8.0);
    
    B.data[0][0] = complex_create(2.0, 1.0);
    B.data[0][1] = complex_create(4.0, 3.0);
    B.data[1][0] = complex_create(6.0, 5.0);
    B.data[1][1] = complex_create(8.0, 7.0);
    
    Matrix result = matrix_add(A, B);
    
    // Expected result
    Matrix expected = matrix_zero(2, 2);
    expected.data[0][0] = complex_create(3.0, 3.0);
    expected.data[0][1] = complex_create(7.0, 7.0);
    expected.data[1][0] = complex_create(11.0, 11.0);
    expected.data[1][1] = complex_create(15.0, 15.0);
    
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
    A.data[0][0] = complex_create(1.0, 0.0);
    A.data[0][1] = complex_create(2.0, 0.0);
    A.data[0][2] = complex_create(3.0, 0.0);
    A.data[1][0] = complex_create(4.0, 0.0);
    A.data[1][1] = complex_create(5.0, 0.0);
    A.data[1][2] = complex_create(6.0, 0.0);
    
    B.data[0][0] = complex_create(7.0, 0.0);
    B.data[0][1] = complex_create(8.0, 0.0);
    B.data[1][0] = complex_create(9.0, 0.0);
    B.data[1][1] = complex_create(10.0, 0.0);
    B.data[2][0] = complex_create(11.0, 0.0);
    B.data[2][1] = complex_create(12.0, 0.0);
    
    Matrix result = matrix_mult(A, B);
    
    // Expected result
    Matrix expected = matrix_zero(2, 2);
    expected.data[0][0] = complex_create(58.0, 0.0);   // 1*7 + 2*9 + 3*11
    expected.data[0][1] = complex_create(64.0, 0.0);   // 1*8 + 2*10 + 3*12
    expected.data[1][0] = complex_create(139.0, 0.0);  // 4*7 + 5*9 + 6*11
    expected.data[1][1] = complex_create(154.0, 0.0);  // 4*8 + 5*10 + 6*12
    
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
    A.data[0][0] = complex_create(1.0, 0.0);
    A.data[0][1] = complex_create(2.0, 0.0);
    A.data[1][0] = complex_create(3.0, 0.0);
    A.data[1][1] = complex_create(4.0, 0.0);
    
    B.data[0][0] = complex_create(5.0, 0.0);
    B.data[0][1] = complex_create(6.0, 0.0);
    B.data[1][0] = complex_create(7.0, 0.0);
    B.data[1][1] = complex_create(8.0, 0.0);
    
    Matrix result = matrix_tensor_product(A, B);
    
    // Expected result (4x4 matrix)
    Matrix expected = matrix_zero(4, 4);
    expected.data[0][0] = complex_create(5.0, 0.0);   // 1*5
    expected.data[0][1] = complex_create(6.0, 0.0);   // 1*6
    expected.data[0][2] = complex_create(10.0, 0.0);  // 2*5
    expected.data[0][3] = complex_create(12.0, 0.0);  // 2*6
    
    expected.data[1][0] = complex_create(7.0, 0.0);   // 1*7
    expected.data[1][1] = complex_create(8.0, 0.0);   // 1*8
    expected.data[1][2] = complex_create(14.0, 0.0);  // 2*7
    expected.data[1][3] = complex_create(16.0, 0.0);  // 2*8
    
    expected.data[2][0] = complex_create(15.0, 0.0);  // 3*5
    expected.data[2][1] = complex_create(18.0, 0.0);  // 3*6
    expected.data[2][2] = complex_create(20.0, 0.0);  // 4*5
    expected.data[2][3] = complex_create(24.0, 0.0);  // 4*6
    
    expected.data[3][0] = complex_create(21.0, 0.0);  // 3*7
    expected.data[3][1] = complex_create(24.0, 0.0);  // 3*8
    expected.data[3][2] = complex_create(28.0, 0.0);  // 4*7
    expected.data[3][3] = complex_create(32.0, 0.0);  // 4*8
    
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
    A.data[0][0] = complex_create(1.0, 2.0);
    A.data[0][1] = complex_create(3.0, 4.0);
    A.data[1][0] = complex_create(5.0, 6.0);
    A.data[1][1] = complex_create(7.0, 8.0);
    
    B.data[0][0] = complex_create(2.0, 1.0);
    B.data[0][1] = complex_create(4.0, 3.0);
    B.data[1][0] = complex_create(6.0, 5.0);
    B.data[1][1] = complex_create(8.0, 7.0);
    
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
    expected.data[0][0] = complex_create(3.0, 3.0);
    expected.data[0][1] = complex_create(7.0, 7.0);
    expected.data[1][0] = complex_create(11.0, 11.0);
    expected.data[1][1] = complex_create(15.0, 15.0);
    
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
    A.data[0][0] = complex_create(1.0, 0.0);
    A.data[0][1] = complex_create(2.0, 0.0);
    A.data[0][2] = complex_create(3.0, 0.0);
    A.data[1][0] = complex_create(4.0, 0.0);
    A.data[1][1] = complex_create(5.0, 0.0);
    A.data[1][2] = complex_create(6.0, 0.0);
    
    B.data[0][0] = complex_create(7.0, 0.0);
    B.data[0][1] = complex_create(8.0, 0.0);
    B.data[1][0] = complex_create(9.0, 0.0);
    B.data[1][1] = complex_create(10.0, 0.0);
    B.data[2][0] = complex_create(11.0, 0.0);
    B.data[2][1] = complex_create(12.0, 0.0);
    
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
    expected.data[0][0] = complex_create(58.0, 0.0);   // 1*7 + 2*9 + 3*11
    expected.data[0][1] = complex_create(64.0, 0.0);   // 1*8 + 2*10 + 3*12
    expected.data[1][0] = complex_create(139.0, 0.0);  // 4*7 + 5*9 + 6*11
    expected.data[1][1] = complex_create(154.0, 0.0);  // 4*8 + 5*10 + 6*12
    
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
    A.data[0][0] = complex_create(1.0, 0.0);
    A.data[0][1] = complex_create(2.0, 0.0);
    A.data[1][0] = complex_create(3.0, 0.0);
    A.data[1][1] = complex_create(4.0, 0.0);
    
    B.data[0][0] = complex_create(5.0, 0.0);
    B.data[0][1] = complex_create(6.0, 0.0);
    B.data[1][0] = complex_create(7.0, 0.0);
    B.data[1][1] = complex_create(8.0, 0.0);
    
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
    expected.data[0][0] = complex_create(5.0, 0.0);   // 1*5
    expected.data[0][1] = complex_create(6.0, 0.0);   // 1*6
    expected.data[0][2] = complex_create(10.0, 0.0);  // 2*5
    expected.data[0][3] = complex_create(12.0, 0.0);  // 2*6
    
    expected.data[1][0] = complex_create(7.0, 0.0);   // 1*7
    expected.data[1][1] = complex_create(8.0, 0.0);   // 1*8
    expected.data[1][2] = complex_create(14.0, 0.0);  // 2*7
    expected.data[1][3] = complex_create(16.0, 0.0);  // 2*8
    
    expected.data[2][0] = complex_create(15.0, 0.0);  // 3*5
    expected.data[2][1] = complex_create(18.0, 0.0);  // 3*6
    expected.data[2][2] = complex_create(20.0, 0.0);  // 4*5
    expected.data[2][3] = complex_create(24.0, 0.0);  // 4*6
    
    expected.data[3][0] = complex_create(21.0, 0.0);  // 3*7
    expected.data[3][1] = complex_create(24.0, 0.0);  // 3*8
    expected.data[3][2] = complex_create(28.0, 0.0);  // 4*7
    expected.data[3][3] = complex_create(32.0, 0.0);  // 4*8
    
    int passed = matrix_compare(A, expected);
    print_test_result("Matrix Tensor Product Override", passed);
    
    matrix_free(A);
    matrix_free(B);
    matrix_free(A_copy);
    matrix_free(expected);
}

int main() {
    printf("Testing Matrix Library with Complex Numbers:\n");
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