#ifndef UTILS_H
#define UTILS_H

#include <complex.h>
#include <stddef.h>
#include <stdint.h>

#define M_PI       3.14159265358979323846
#define EPSILON    1e-9

typedef unsigned long int int64;

double now_seconds();

int64 gcd(int64 a, int64 b);
int64 lcm(int64 a, int64 b);

void bezout_coef(long long a, long long b, long long *u, long long *v);
int64 get_inverse(int64 a, int64 N);

int64 fexp(int64 x, int64 n);
int64 fexp_mod(int64 x, int64 n, int64 N);

void *malloc_custom(int64 size);
void *calloc_custom(int64 amount, int64 element_size);
void free_custom(void *ptr);

/**
 * Robust bit spreading for a contiguous window of qubits.
 * Inserts 'window_val' (nb bits) into 'base_val' at position 'start'.
 */
int64 spread_bits(int64 base_val, int64 window_val, int start, int nb);
#endif