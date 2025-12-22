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

int fast_exp_i(int x, int n) {
    if(n == 0) return 1;
    else if (n == 1) return x;
    else if (n % 2 == 0) {
        int res = fast_exp_i(x, n/2);
        return res * res;
    }
    else {
        int res = fast_exp_i(x, (n-1)/2);
        return res * res * x;
    }
}

double fast_exp_d(double x, int n) {
    if(n == 0) return 1;
    else if (n == 1) return x;
    else if (n % 2 == 0) {
        double res = fast_exp_d(x, n/2);
        return res * res;
    }
    else {
        double res = fast_exp_d(x, (n-1)/2);
        return res * res * x;
    }
}

void print_list(int *bits, int n) {
    printf("[");
    for(int i = 0; i < n; i++) {
        if (i < n - 1) printf("%d, ", bits[i]);
        else printf("%d]\n", bits[i]);
    }
}

void print_state(double complex *state, int n) {
    printf("[");
    for(int i = 0; i < n; i++) {
        if (i < n - 1) printf("%.2f + i%.2f, ", creal(state[i]), cimag(state[i]));
        else printf("%.2f + i%.2f]\n", creal(state[i]), cimag(state[i]));
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

