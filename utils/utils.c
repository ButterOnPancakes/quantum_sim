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
    void *memory = malloc(size);
    assert(memory != NULL);
    return memory;
}
void *calloc_custom(size_t elementCount, size_t elementSize) {
    void *memory = calloc(elementCount, elementSize);
    assert(memory != NULL);
    return memory;
}