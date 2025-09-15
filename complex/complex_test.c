#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "complexes.h"
#include "../utils/utils.h"

// Test utilities
#define TOLERANCE 1e-15

int complex_compare(Complex a, Complex b) {
    return fabs(a.real - b.real) < TOLERANCE && 
           fabs(a.imag - b.imag) < TOLERANCE;
}

void test_complex_create() {
    Complex z = complex_create(3.0, 4.0);
    printf("Create Test: %s\n", 
           (z.real == 3.0 && z.imag == 4.0) ? "PASS" : "FAIL");
}

void test_complex_norm() {
    Complex z = {3.0, 4.0};
    double norm = complex_norm(z);
    printf("Norm Test: %s\n", 
           fabs(norm - 5.0) < TOLERANCE ? "PASS" : "FAIL");
}

void test_complex_conjugate() {
    Complex z = {3.0, 4.0};
    Complex conj = complex_conjugate(z);
    Complex expected = {3.0, -4.0};
    printf("Conjugate Test: %s\n", 
           complex_compare(conj, expected) ? "PASS" : "FAIL");
}

void test_complex_scale() {
    Complex z = {3.0, 4.0};
    Complex scaled = complex_scale(z, 2.0);
    Complex expected = {6.0, 8.0};
    printf("Scale Test: %s\n", 
           complex_compare(scaled, expected) ? "PASS" : "FAIL");
}

void test_complex_exp() {
    Complex z = complex_exp(2.0, M_PI); // Should be -2.0 + 0i
    Complex expected = {-2.0, 0.0};
    printf("Exp Test: %s\n", 
           complex_compare(z, expected) ? "PASS" : "FAIL");
}

void test_complex_add() {
    Complex a = {1.0, 2.0};
    Complex b = {3.0, 4.0};
    Complex result = complex_add(a, b);
    Complex expected = {4.0, 6.0};
    printf("Add Test: %s\n", 
           complex_compare(result, expected) ? "PASS" : "FAIL");
}

void test_complex_sub() {
    Complex a = {1.0, 2.0};
    Complex b = {3.0, 4.0};
    Complex result = complex_sub(a, b);
    Complex expected = {-2.0, -2.0};
    printf("Sub Test: %s\n", 
           complex_compare(result, expected) ? "PASS" : "FAIL");
}

void test_complex_mult() {
    Complex a = {1.0, 2.0};
    Complex b = {3.0, 4.0};
    Complex result = complex_mult(a, b);
    Complex expected = {-5.0, 10.0}; // (1+2i)*(3+4i) = -5+10i
    printf("Mult Test: %s\n", 
           complex_compare(result, expected) ? "PASS" : "FAIL");
}

void test_complex_div() {
    Complex a = {1.0, 2.0};
    Complex b = {3.0, 4.0};
    Complex result = complex_div(a, b);
    Complex expected = {0.44, 0.08}; // (1+2i)/(3+4i) = 0.44 + 0.08i
    printf("Div Test: %s\n", 
           complex_compare(result, expected) ? "PASS" : "FAIL");
}

int main() {
    test_complex_create();
    test_complex_norm();
    test_complex_conjugate();
    test_complex_scale();
    test_complex_exp();
    test_complex_add();
    test_complex_sub();
    test_complex_mult();
    test_complex_div();
    return 0;
}