#include "fft.h"
#include "utils.h"
#include <math.h>
#include <stdint.h>

static uint64_t reverse_bits(uint64_t x, int n) {
    uint64_t res = 0;
    for (int i = 0; i < n; i++) {
        res = (res << 1) | (x & 1);
        x >>= 1;
    }
    return res;
}

void fft_base(float complex *x, uint64_t n, bool inverse) {
    int levels = 0;
    while (((uint64_t)1 << levels) < n) levels++;

    for (uint64_t i = 0; i < n; i++) {
        uint64_t j = reverse_bits(i, levels);
        if (i < j) {
            float complex temp = x[i];
            x[i] = x[j];
            x[j] = temp;
        }
    }

    for (int s = 1; s <= levels; s++) {
        uint64_t m = (uint64_t)1 << s;
        uint64_t m2 = m >> 1;
        float complex w_m = cexp((inverse ? 2.0 : -2.0) * M_PI * I / (double)m);
        for (uint64_t k = 0; k < n; k += m) {
            float complex w = 1.0;
            for (uint64_t j = 0; j < m2; j++) {
                float complex t = w * x[k + j + m2];
                float complex u = x[k + j];
                x[k + j] = u + t;
                x[k + j + m2] = u - t;
                w *= w_m;
            }
        }
    }
}

void fft(float complex *x, uint64_t n) {
    fft_base(x, n, false);
}

void ifft(float complex *x, uint64_t n) {
    fft_base(x, n, true);
    for (uint64_t i = 0; i < n; i++) x[i] /= (double)n;
}

void qft_base(float complex *x, uint64_t n, bool inverse) {
    fft_base(x, n, !inverse);
    
    float complex scale = 1.0 / sqrt((double)n);
    for (uint64_t i = 0; i < n; i++) x[i] *= scale;
}