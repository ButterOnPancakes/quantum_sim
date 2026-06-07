#ifndef FFT_H
#define FFT_H

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>

void fft(float complex *x, uint64_t n);
void ifft(float complex *x, uint64_t n);
void qft_base(float complex *x, uint64_t n, bool inverse);

#endif