#ifndef COMPLEX_H
#define COMPLEX_H

typedef struct complex {
    double real;
    double imag;
} Complex;

Complex complex_create(double real, double imag);

void complex_print(Complex a);

double complex_norm(Complex a);
Complex complex_conjugate(Complex a);
Complex complex_scale(Complex a, double scalar);
Complex complex_exp(double magnitude, double angle);

Complex complex_add(Complex a, Complex b);
Complex complex_sub(Complex a, Complex b);
Complex complex_mult(Complex a, Complex b);
Complex complex_div(Complex a, Complex b);

#endif