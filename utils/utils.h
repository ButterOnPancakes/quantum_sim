#ifndef UTILS_H
#define UTILS_H

#include <complex.h>
#include <stddef.h>
#include <stdint.h>

#define M_PI       3.14159265358979323846
#define EPSILON    1e-9

double now_seconds();
void *malloc_custom(size_t size);
void *calloc_custom(size_t amount, size_t element_size);

/**
 * Robust bit spreading for a contiguous window of qubits.
 * Inserts 'window_val' (nb bits) into 'base_val' at position 'start'.
 */
static inline uint64_t spread_bits(uint64_t base_val, uint64_t window_val, int start, int nb) {
    if (start == 0) return (base_val << nb) | window_val;
    
    uint64_t low_mask = (1ULL << start) - 1;
    uint64_t low_bits = base_val & low_mask;
    uint64_t high_bits = (base_val >> start) << (start + nb);
    return high_bits | low_bits | (window_val << start);
}

#endif