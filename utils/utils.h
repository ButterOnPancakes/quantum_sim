#ifndef UTILS_H
#define UTILS_H

#include <complex.h>
#include <stddef.h>

#define M_PI       3.14159265358979323846

double now_seconds();
int reverse_power(int N);

void *malloc_custom(size_t size);
void *calloc_custom(size_t nmemb, size_t size);
void free_custom(void *ptr);

void print_int_array(int *bits, int n);

void *aligned_alloc_64(size_t size);

#endif