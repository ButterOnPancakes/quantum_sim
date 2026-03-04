#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

double now_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void *malloc_custom(size_t size) {
    void *ptr = malloc(size);
    assert(ptr != NULL);
    return ptr;
}

void *calloc_custom(size_t amount, size_t element_size) {
    void *ptr = calloc(amount, element_size);
    assert(ptr != NULL);
    return ptr;
}