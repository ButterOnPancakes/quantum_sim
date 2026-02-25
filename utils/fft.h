#ifndef FFT_H
#define FFT_H

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>

void fft(double complex *x, uint64_t n);
void ifft(double complex *x, uint64_t n);
void qft_base(double complex *x, uint64_t n, bool inverse);

#endif