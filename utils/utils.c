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

int gcd(int a, int b) {
    if (b == 0) return a;
    if(a < b) return gcd(b, a);
    return gcd(b, a % b);
}
int lcm(int a, int b) {
    return (a * b) / gcd(a, b);
}

// ri_1 = r(i-1)
void bezout_coef_rec(int *ri_2, int *ri_1, int *ui_2, int *ui_1, int *vi_2, int *vi_1) {
    int ri = *ri_2 % *ri_1;
    int qi_1 = *ri_2 / *ri_1;

    int ui = *ui_2 - qi_1 * *ui_1;
    int vi = *vi_2 - qi_1 * *vi_1;

    *ri_2 = *ri_1; *ri_1 = ri;
    *ui_2 = *ui_1; *ui_1 = ui;
    *vi_2 = *vi_1; *vi_1 = vi;

    if(ri == 0) return;
    else bezout_coef_rec(ri_2, ri_1, ui_2, ui_1, vi_2, vi_1);
}
void bezout_coef(int a, int b, int *u, int *v) {
    int r0 = a, r1 = b;
    int u0 = 1, u1 = 0;
    int v0 = 0, v1 = 1;
    bezout_coef_rec(&r0, &r1, &u0, &u1, &v0, &v1);

    *u = u0; *v = v0;
}
int get_inverse(int a, int N) {
    int u,v;
    bezout_coef(a, N, &u, &v);

    return u % N;
}

int fexp(int x, int n) {
    if(n == 0) return 1;
    if(n%2 == 0) {
        int t = fexp(x, n/2);
        return t*t;
    }
    else {
        int t = fexp(x, (n-1)/2);
        return x*t*t;
    }
}
int fexp_mod(int x, int n, int N) {
    if(n == 0) return 1;
    if(n%2 == 0) {
        int t = fexp_mod(x, n/2, N) % N;
        return t*t % N;
    }
    else {
        int t = fexp_mod(x, (n-1)/2, N) % N;
        return x*t*t % N;
    }
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
void free_custom(void *ptr) {
    assert(ptr != NULL);
    free(ptr);
}

uint64_t spread_bits(uint64_t base_val, uint64_t window_val, int start, int nb) {
    if (start == 0) return (base_val << nb) | window_val;
    
    uint64_t low_mask = (1ULL << start) - 1;
    uint64_t low_bits = base_val & low_mask;
    uint64_t high_bits = (base_val >> start) << (start + nb);
    return high_bits | low_bits | (window_val << start);
}