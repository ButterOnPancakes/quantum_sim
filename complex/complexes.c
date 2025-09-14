#include "complexes.h"

typedef struct complex {
    double real;
    double imag;
} Complex;

Complex complex_create(double real, double imag) {
    Complex a;
    a.real = real;
    a.imag = imag;
    return a;
}

void complex_print(Complex a) {
    if (a.imag >= 0) printf("%.3f + %.3fi\n", a.real, a.imag);
    else printf("%.3f - %.3fi\n", a.real, -a.imag);
}

double complex_norm(Complex a) {
    return sqrt(a.real * a.real + a.imag * a.imag);
}
Complex complex_conjugate(Complex a) {
    return complex_create(a.real, -a.imag);
}
Complex complex_scale(Complex a, double scalar) {
    return complex_create(a.real * scalar, a.imag * scalar);
}
Complex complex_exp(double norm, double angle) {
    return complex_create(norm * cos(angle), norm * sin(angle));
}

Complex complex_add(Complex a, Complex b) {
    return complex_create(a.real + b.real, a.imag + b.imag);
}
Complex complex_sub(Complex a, Complex b) {
    return complex_create(a.real - b.real, a.imag - b.imag);
}
Complex complex_mult(Complex a, Complex b) {
    return complex_create(a.real * b.real - a.imag * b.imag, a.real * b.imag + a.imag * b.real);
}
Complex complex_div(Complex a, Complex b) {
    double denom = b.real * b.real + b.imag * b.imag;
    return complex_create((a.real * b.real + a.imag * b.imag) / denom, (a.imag * b.real - a.real * b.imag) / denom);
}