#ifndef UTILS_H
#define UTILS_H

#include <complex.h>
#include <stddef.h>
#include <stdint.h>

#define M_PI       3.14159265358979323846
#define EPSILON    1e-9

double now_seconds();

int gcd(int a, int b);
int lcm(int a, int b);

void bezout_coef(int a, int b, int *u, int *v);
int get_inverse(int a, int N);

int fexp(int x, int n);
int fexp_mod(int x, int n, int N);

void *malloc_custom(size_t size);
void *calloc_custom(size_t amount, size_t element_size);
void free_custom(void *ptr);

/**
 * Robust bit spreading for a contiguous window of qubits.
 * Inserts 'window_val' (nb bits) into 'base_val' at position 'start'.
 */
uint64_t spread_bits(uint64_t base_val, uint64_t window_val, int start, int nb);
#endif