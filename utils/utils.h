#ifndef UTILS_H
#define UTILS_H

#include <complex.h>
#include <stddef.h>

#define M_PI       3.14159265358979323846

double now_seconds();

void *malloc_custom(size_t memorySize);
void *calloc_custom(size_t elementCount, size_t elementSize);

#endif