#include "gates.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <complex.h>
#include <math.h>
#include <time.h>

#include <omp.h>

void gate_h(double complex g[4]) {
    double s = 1.0 / sqrt(2.0);
    g[0] = s; g[1] = s;
    g[2] = s; g[3] = -s;
}
void gate_x(double complex g[4]) {
    g[0] = 0.0; g[1] = 1.0;
    g[2] = 1.0; g[3] = 0.0;
}
void gate_z(double complex g[4]) {
    g[0] = 1.0; g[1] = 0.0;
    g[2] = 0.0; g[3] = -1.0;
}

void apply_single_qubit_inplace(double complex *state, int nqubits, int t, double complex g[4]) {
    int size = 1 << nqubits;
    int bit = 1 << t;

    #pragma omp parallel for schedule(static)
}

void apply_two_qubit_inplace(double complex *state, int nqubits, int q0, int q1, double complex G[16]);

void apply_controlled_u_inplace(double complex *state, int nqubits, int c, int t, double complex U[4]);

int measure_qubit_inplace(double complex *state, int nqubits, int t);