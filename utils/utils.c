#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double now_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int reverse_power(int N) {
    int i = 0;
    while(N > 0) {
        N = N >> 1;
        i++;
    }
    return i - 1;
}

void print_int_array(int *bits, int n) {
    printf("[");
    for(int i = 0; i < n; i++) {
        if (i < n - 1) printf("%d, ", bits[i]);
        else printf("%d]\n", bits[i]);
    }
}

void *aligned_alloc_64(size_t size) {
    void *p = NULL;
    // POSIX aligned_alloc (size must be multiple of alignment)
    size_t align = 64;
    size_t sz = ((size + align - 1) / align) * align;
    p = aligned_alloc(align, sz);
    if (!p) {
        // fallback to posix_memalign
        if (posix_memalign(&p, align, sz) != 0) p = NULL;
    }
    return p;
}

