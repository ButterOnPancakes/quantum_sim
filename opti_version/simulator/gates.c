#include "gates.h"

#include "../builder/circuit.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <complex.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include <omp.h>

void apply_corresponding_gate(double complex g[4], SingleBitGate gt, double phase) {
    switch(gt) {
        case GATE_I: break;
        case GATE_H: gate_h(g); break;
        case GATE_X: gate_x(g); break;
        case GATE_Y: gate_y(g); break;
        case GATE_Z: gate_z(g); break;
        case GATE_PHASE: gate_phase(g, phase); break;
    }
}

void gate_h(double complex g[4]) {
    double s = 1.0 / sqrt(2.0);
    g[0] = s; g[1] = s;
    g[2] = s; g[3] = -s;
}
void gate_x(double complex g[4]) {
    g[0] = 0.0; g[1] = 1.0;
    g[2] = 1.0; g[3] = 0.0;
}
void gate_y(double complex g[4]) {
    g[0] = 0.0; g[1] = -I;
    g[2] = I; g[3] = 0.0;
}
void gate_z(double complex g[4]) {
    g[0] = 1.0; g[1] = 0.0;
    g[2] = 0.0; g[3] = -1.0;
}

void gate_phase(double complex g[4], double phase) {
    g[0] = 1.0; g[1] = 0.0;
    g[2] = 0.0; g[3] = cexp(I * phase);
}

uint64_t get_bit(uint64_t x, int pos, int nqbits) {
    return (x >> (nqbits - 1 - pos)) & 1;
}
uint64_t set_bit(uint64_t x, int pos, int nqbits, int val) {
    int mask = 1 << (nqbits - 1 - pos);
    return val ? (x | mask) : (x & ~mask);
}

void apply_single_qubit_inplace(double complex *state, int nqubits, int k, double complex g[4]) {
    uint64_t size = 1 << nqubits;
    uint64_t bit = 1ULL << (nqubits - k - 1);

    /* Parcours tous les b_1, ..., b_k-1, b_k+1, ..., b_n possibles en
    effectuant la division euclidienne d'un indice par 2^(k-1) :
    i0 = (i = 2^(k+1) * l) + r avec l < 2^(n - k - 1) et r < 2^k -> Terme en 2^k nul */
    
    //#pragma omp parallel for schedule(static)
    for(uint64_t i = 0; i < size; i+= (bit << 1)) {
        for(uint64_t r = 0; r < bit; r++) {
            uint64_t i0 = i + r;
            uint64_t i1 = i0 + bit; // Representation binaire pour b_k = 1
            
            double complex a0 = state[i0];
            double complex a1 = state[i1];
            
            state[i0] = g[0] * a0 + g[1] * a1;
            state[i1] = g[2] * a0 + g[3] * a1;
        }
    }
}
void apply_two_qubit_inplace(double complex *state, int nqubits, int q0, int q1, double complex G[16]) {
    assert(q0 != q1);
    if(q1 < q0) {int temp = q1; q1 = q0; q0 = temp;}

    uint64_t size = 1 << nqubits;
    uint64_t bit0 = 1 << (nqubits - q0 - 1);
    uint64_t bit1 = 1 << (nqubits - q1 - 1);

    /* Même principe : 
    i0 = (k * 2^(q1 + 1)) + (l * 2^(q0+1)) + r
    avec l * 2^(q0+1) < 2^q1 et r < 2^q0
    */
    
    //#pragma omp parallel for schedule(static)
    for(uint64_t k = 0; k < size; k += (bit1 << 1)) {
        for(uint64_t l = 0; l < bit1; l += (bit0 << 1)) {
            for(uint64_t r = 0; r < bit0; r++) {
                uint64_t i00 = k + l + r; // q1=0 q0=0
                uint64_t i01 = i00 + bit0; // q1=0 q0=1
                uint64_t i10 = i00 + bit1; // q1=1 q0=0
                uint64_t i11 = i10 + bit0; // q1=1 q0=1

                double complex v00 = state[i00];
                double complex v01 = state[i01];
                double complex v10 = state[i10];
                double complex v11 = state[i11];

                // multiply: new = G * vec([v00,v01,v10,v11])
                state[i00] = G[0]*v00 + G[1]*v01 + G[2]*v10 + G[3]*v11;
                state[i01] = G[4]*v00 + G[5]*v01 + G[6]*v10 + G[7]*v11;
                state[i10] = G[8]*v00 + G[9]*v01 + G[10]*v10 + G[11]*v11;
                state[i11] = G[12]*v00 + G[13]*v01 + G[14]*v10 + G[15]*v11;
            }
        }
    }
}
void apply_controlled_u_inplace(double complex *state, int nqubits, int c, int t, double complex U[4]) {
    uint64_t size = 1 << nqubits;
    uint64_t control = 1 << (nqubits - c - 1);
    uint64_t target = 1 << (nqubits - t - 1);

    //#pragma omp parallel for schedule(static)
    for(uint64_t base = 0; base < size; base++) {
        /* If control is 1 on this base and target is 0 
        (to make sure that adding 2^(t) does not impact the rest) */
        if((base & control) && !(base & target)) {
            uint64_t i0 = base; // Control = 1, Target = 0
            uint64_t i1 = base + target; // Control = 1, Target = 1

            double complex a0 = state[i0];
            double complex a1 = state[i1];
            
            state[i0] = U[0] * a0 + U[1] * a1;
            state[i1] = U[2] * a0 + U[3] * a1;
        }
    }
}

/* For index = x(0)...x(n) and targets = {t0...tk}, gives the number x(t0)...x(tk)*/
uint64_t extract_subindex(uint64_t index, int *targets, int k, int nqbits) {
    uint64_t sub = 0;
    for (int i = 0; i < k; i++) {
        sub <<= 1;
        sub |= get_bit(index, targets[i], nqbits);
    }
    return sub;
}
/* Replaces the (x(ti)) in x(0)...x(n) by sub*/
uint64_t replace_subindex(uint64_t index, int *targets, int k, int nqbits, uint64_t sub) {
    for (int i = k - 1; i >= 0; i--) {
        int bit = sub & 1;
        sub >>= 1;
        index = set_bit(index, targets[i], nqbits, bit);
    }
    return index;
}

void apply_custom_inplace(double complex *state, int nqbits, int *targets, int k, double complex *U) {
    uint64_t dim = 1 << nqbits;
    uint64_t subdim = 1 << k;

    double complex *new_state = calloc(dim, sizeof(double complex));

    for (uint64_t i = 0; i < dim; i++) {
        uint64_t row = extract_subindex(i, targets, k, nqbits);

        for (uint64_t col = 0; col < subdim; col++) {
            uint64_t j = replace_subindex(i, targets, k, nqbits, col);
            new_state[i] += U[row * subdim + col] * state[j];
        }
    }

    memcpy(state, new_state, dim * sizeof(double complex));
    free(new_state);
}

int measure_qubit_inplace(double complex *state, int nqubits, int t) {
    uint64_t size = 1 << nqubits;
    uint64_t bit = 1 << (nqubits - t - 1);

    // Compute Norm squared of P0 * phi (phi after projection of the bit on zero)
    double p0 = 0.0;
    
    //#pragma omp parallel for reduction(+:p0) schedule(static)
    for (uint64_t i = 0; i < size; ++i) {
        // If the t bit of the basis is 0
        if (!(i & bit)) {
            double re = creal(state[i]), im = cimag(state[i]);
            p0 += re*re + im*im;
        }
    }

    double r = (double)rand() / (double)RAND_MAX;
    int result = (r < p0) ? 0 : 1;
    double keep_prob = (result == 0) ? p0 : (1.0 - p0);
    double norm = (keep_prob <= 0) ? 1.0 : 1.0 / sqrt(keep_prob);
    
    //printf("Measured %d on qbit %d with probability %.2f\n", result, t, keep_prob);
            
    // Collapse the qbit and norm it
    //#pragma omp parallel for schedule(static)
    for (uint64_t i = 0; i < size; ++i) {
        // If the bit of this basis is not the result
        if (((i & bit) ? 1 : 0) != result) {
            state[i] = 0.0 + 0.0*I;
        } else {
            state[i] *= norm;
        }
    }

    return result;
}