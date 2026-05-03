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

int64 gcd(int64 a, int64 b) {
    if (b == 0) return a;
    if(a < b) return gcd(b, a);
    return gcd(b, a % b);
}
int64 lcm(int64 a, int64 b) {
    return (a / gcd(a, b)) * b;
}

// ri_1 = r(i-1)
void bezout_coef_rec(long long *ri_2, long long *ri_1, long long *ui_2, long long *ui_1, long long *vi_2, long long *vi_1) {
    long long ri = *ri_2 % *ri_1;
    long long qi_1 = *ri_2 / *ri_1;

    long long ui = *ui_2 - qi_1 * *ui_1;
    long long vi = *vi_2 - qi_1 * *vi_1;

    *ri_2 = *ri_1; *ri_1 = ri;
    *ui_2 = *ui_1; *ui_1 = ui;
    *vi_2 = *vi_1; *vi_1 = vi;

    if(ri == 0) return;
    else bezout_coef_rec(ri_2, ri_1, ui_2, ui_1, vi_2, vi_1);
}
void bezout_coef(long long a, long long b, long long *u, long long *v) {
    long long r0 = a, r1 = b;
    long long u0 = 1, u1 = 0;
    long long v0 = 0, v1 = 1;
    bezout_coef_rec(&r0, &r1, &u0, &u1, &v0, &v1);

    *u = u0; *v = v0;
}
int64 get_inverse(int64 a, int64 N) {
    long long u,v;
    bezout_coef(a, N, &u, &v);
    
    return ((u % (long long)N) + (long long)N) % (long long)N;
}

int64 fexp(int64 x, int64 n) {
    if(n == 0) return 1;
    if(n%2 == 0) {
        int64 t = fexp(x, n/2);
        return t*t;
    }
    else {
        int64 t = fexp(x, (n-1)/2);
        return x*t*t;
    }
}
int64 fexp_mod(int64 x, int64 n, int64 N) {
    if(n == 0) return 1;
    if(n%2 == 0) {
        int64 t = fexp_mod(x, n/2, N) % N;
        return t*t % N;
    }
    else {
        int64 t = fexp_mod(x, (n-1)/2, N) % N;
        return x*t*t % N;
    }
}

void *malloc_custom(int64 size) {
    void *ptr = malloc(size);
    assert(ptr != NULL);
    return ptr;
}
void *calloc_custom(int64 amount, int64 element_size) {
    void *ptr = calloc(amount, element_size);
    assert(ptr != NULL);
    return ptr;
}
void free_custom(void *ptr) {
    assert(ptr != NULL);
    free(ptr);
}

int64 spread_bits(int64 base_val, int64 window_val, int start, int nb) {
    if (start == 0) return (base_val << nb) | window_val;
    
    int64 low_mask = (1ULL << start) - 1;
    int64 low_bits = base_val & low_mask;
    int64 high_bits = (base_val >> start) << (start + nb);
    return high_bits | low_bits | (window_val << start);
}