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

void apply_single_qubit_inplace(double complex *state, int nqubits, int k, double complex g[4]) {
    uint64_t size = 1 << nqubits;
    uint64_t add_i = 1 << (nqubits - k);
    uint64_t sup_r = 1 << (nqubits - k - 2);
    if(nqubits - k - 2 < 0) sup_r = 0;

    /* Parcours tous les b_1, ..., b_k-1, b_k+1, ..., b_n possibles en
    effectuant la division euclidienne d'un indice par 2^(n-k) :
    i0 = (i = 2^(n-k) * l) + r avec l < 2^k et r < 2^(n - k - 2) -> Terme en 2^(n-k-1) nul */
    #pragma omp parallel for schedule(static)
    for(uint64_t i = 0; i < size; i+= add_i) {
        for(uint64_t r = 0; r < sup_r; r++) {
            uint64_t i0 = i + r;
            uint64_t i1 = i0 + 2 * sup_r; // Representation binaire en b_k * 2^(n-k-1)
            
            double complex a0 = state[i0];
            double complex a1 = state[i1];
            
            state[i0] = g[0] * a0 + g[1] * a1;
            state[i1] = g[2] * a0 + g[3] * a1;
        }
    }
}

void apply_two_qubit_inplace(double complex *state, int nqubits, int q0, int q1, double complex G[16]);

void apply_controlled_u_inplace(double complex *state, int nqubits, int c, int t, double complex U[4]);

int measure_qubit_inplace(double complex *state, int nqubits, int t);