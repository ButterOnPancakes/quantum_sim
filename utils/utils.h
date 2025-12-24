#ifndef UTILS_H
#define UTILS_H

#include <complex.h>
#include <stddef.h>

#define M_PI       3.14159265358979323846

double now_seconds();
int reverse_power(int N);

int fast_exp_i(int x, int n);
double fast_exp_d(double x, int n);

void print_int_array(int *bits, int n);
void print_complex_array(double complex *state, int n);

void *aligned_alloc_64(size_t size);

#endif