#ifndef FFT_H
#define FFT_H

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>

#include "../utils/utils.h"

void qft_base(double complex *x, int64 n, bool inverse);

#endif