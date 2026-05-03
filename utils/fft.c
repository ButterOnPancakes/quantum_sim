#include "fft.h"

#include "utils.h"

#include <math.h>
#include <stdint.h>

static int64 reverse_bits(int64 x, int n) {
    int64 res = 0;
    for (int i = 0; i < n; i++) {
        res = (res << 1) | (x & 1);
        x >>= 1;
    }
    return res;
}

void compute_fft(double complex *x, int64 n, bool inverse) {
    // Cooley-Tukey FFT algorithm
    int levels = 0;
    while (((int64)1ULL << levels) < n) levels++;

    for (int64 i = 0; i < n; i++) {
        int64 j = reverse_bits(i, levels);
        if (i < j) {
            double complex temp = x[i];
            x[i] = x[j];
            x[j] = temp;
        }
    }

    for (int s = 1; s <= levels; s++) {
        int64 m = (int64)1ULL << s;
        int64 m2 = m >> 1;
        double complex w_m = cexp((inverse ? 2.0 : -2.0) * M_PI * I / (double)m);
        for (int64 k = 0; k < n; k += m) {
            double complex w = 1.0;
            for (int64 j = 0; j < m2; j++) {
                double complex t = w * x[k + j + m2];
                double complex u = x[k + j];
                x[k + j] = u + t;
                x[k + j + m2] = u - t;
                w *= w_m;
            }
        }
    }
}

void qft_base(double complex *x, int64 n, bool inverse) {
    compute_fft(x, n, !inverse);
    
    double complex scale = 1.0 / sqrt((double)n);
    for (int64 i = 0; i < n; i++) x[i] *= scale;
}