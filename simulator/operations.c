#include "operations.h"
#include "../builder/registers.h"
#include "../builder/registers_internal.h"
#include "../utils/utils.h"
#include "../utils/fft.h"

#include <stdio.h>
#include <stdbool.h>
#include <complex.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <omp.h>

/*
 * Per-gate OpenMP parallelism is intentionally disabled here.
 * The thread-fork/join overhead dominates for individual gate loops.
 * Coarse-grained parallelism is applied at the circuit level instead
 * (see order_finding.c: #pragma omp parallel for over phase-estimation runs).
 */

// 1-qbit gate, issu de opti_version
void apply_hadamard(QuantumRegister *qreg, int qbit) {
    int64 size = 1ULL << qreg->nb_qbits;
    int64 bit = 1ULL << qbit; // Little Endian: bit 0 is the 2^0 position
    
    double inv_sqrt2 = sqrt(0.5);

    #if USE_OMP
    #pragma omp parallel for
    #endif
    for (int64 i = 0; i < size; i++) {
        if(!(i & bit)) {
            int64 i0 = i;
            int64 i1 = i0 | bit;
            
            double complex a0 = qreg->array[i0];
            double complex a1 = qreg->array[i1];
            
            qreg->array[i0] = (a0 + a1) * inv_sqrt2;
            qreg->array[i1] = (a0 - a1) * inv_sqrt2;
        }
    }
}
void apply_gate_x(QuantumRegister *qreg, int qbit) {
    int64 size = 1ULL << qreg->nb_qbits;
    int64 bit = 1ULL << qbit;
    
    #if USE_OMP
    #pragma omp parallel for
    #endif
    for (int64 i = 0; i < size; i++) {
        if(!(i & bit)) {
            int64 i0 = i;
            int64 i1 = i0 | bit;
            
            double complex tmp = qreg->array[i0];
            qreg->array[i0] = qreg->array[i1];
            qreg->array[i1] = tmp;
        }
    }
}
void apply_gate_y(QuantumRegister *qreg, int qbit) {
    int64 size = 1ULL << qreg->nb_qbits;
    int64 bit = 1ULL << qbit;
    
    #if USE_OMP
    #pragma omp parallel for
    #endif
    for (int64 i = 0; i < size; i++) {
        if(!(i & bit)) {
            int64 i0 = i;
            int64 i1 = i0 | bit;
            
            double complex a0 = qreg->array[i0];
            double complex a1 = qreg->array[i1];
            qreg->array[i0] = -I * a1; 
            qreg->array[i1] =  I * a0;
        }
    }
}
void apply_gate_z(QuantumRegister *qreg, int qbit) {
    int64 size = 1ULL << qreg->nb_qbits;
    int64 bit = 1ULL << qbit;
    
    #if USE_OMP 
    #pragma omp parallel for
    #endif
    for (int64 i = 0; i < size; i++) {
        if(i & bit) {
            qreg->array[i] *= -1.0;
        }
    }
}
void apply_gate_phase(QuantumRegister *qreg, int qbit, double phase) {
    int64 size = 1ULL << qreg->nb_qbits;
    int64 bit = 1ULL << qbit;
    double complex expo = cexp(I * phase);
    
    #if USE_OMP
    #pragma omp parallel for
    #endif
    for (int64 i = 0; i < size; i++) {
        if(i & bit) {
            qreg->array[i] *= expo;
        }
    }
}

// 2-qbit gates, issu de opti_version
void apply_cnot(QuantumRegister *qreg, int c, int t) {
    int64 size = 1ULL << qreg->nb_qbits;
    int64 c_bit = 1ULL << c;
    int64 t_bit = 1ULL << t;

    // We only need to iterate through half the register (where control bit is 1)
    #if USE_OMP
    #pragma omp parallel for
    #endif
    for (int64 i = 0; i < size; i++) {
        if ((i & c_bit) && !(i & t_bit)) {
            int64 i0 = i;
            int64 i1 = i | t_bit;
            
            double complex tmp = qreg->array[i0];
            
            qreg->array[i0] = qreg->array[i1];
            qreg->array[i1] = tmp;
        }
    }
}
void apply_controlled_rotation(QuantumRegister *qreg, int c, int t, double phase) {
    int64 size = 1ULL << qreg->nb_qbits;
    int64 c_bit = 1ULL << c;
    int64 t_bit = 1ULL << t;

    double complex expo = cexp(I * phase);

    // Act on control_bit = 1 and target_bit = 1

    #if USE_OMP
    #pragma omp parallel for
    #endif
    for (int64 i = 0; i < size; i++) {
        if ((i & c_bit) && (i & t_bit)) {
            qreg->array[i] *= expo;
        }
    }
}
void apply_swap(QuantumRegister *qreg, int q0, int q1) {
    int64 size = 1ULL << qreg->nb_qbits;
    int64 b0 = 1ULL << q0;
    int64 b1 = 1ULL << q1;

    #if USE_OMP
    #pragma omp parallel for
    #endif
    for (int64 i = 0; i < size; i++) {
        // Only swap if bits at q0 and q1 are different (01 or 10)
        // We act when bit q0=1 and q1=0 to perform the swap once
        if ((i & b0) && !(i & b1)) {
            int64 i_01 = (i & ~b0) | b1; // This is the state where bits are swapped
            double complex tmp = qreg->array[i];
            qreg->array[i] = qreg->array[i_01];
            qreg->array[i_01] = tmp;
        }
    }
}

// n-qbit gates
void apply_oracle_phase(QuantumRegister *qreg, int start_qbit, int nb_qbits, bool (*function)(int64 number)) {
    int64 size = qreg->size;
    int64 input_mask = (1ULL << nb_qbits) - 1;

    #if USE_OMP
    #pragma omp parallel for
    #endif
    for (int64 i = 0; i < size; i++) {
        int64 number = (i >> start_qbit) & input_mask;

        if (function(number)) {
            qreg->array[i] = -qreg->array[i];
        }
    }
}
void apply_oracle_ancilla(QuantumRegister *qreg, int start_qbit, int nb_qbits, int ancilla_qbit, bool (*function)(int64 number)) {
    int64 size = 1ULL << qreg->nb_qbits;
    int64 ancilla_bit = 1ULL << ancilla_qbit; // Little Endian
    int64 input_mask = (1ULL << nb_qbits) - 1;

    #if USE_OMP
    #pragma omp parallel for
    #endif
    for (int64 i = 0; i < size; i++) {
        // We only process the pair when the ancilla bit is 0
        // This ensures we only perform the swap once per pair
        if (!(i & ancilla_bit)) {
            int64 number = (i >> start_qbit) & input_mask;

            if (function(number)) {
                int64 i0 = i;
                int64 i1 = i | ancilla_bit;

                double complex temp = qreg->array[i0];
                qreg->array[i0] = qreg->array[i1];
                qreg->array[i1] = temp;
            }
        }
    }
}

void apply_diffusion(QuantumRegister *qreg, int start_qbit, int nb_qbits) {
    int64 total_size = qreg->size;
    int64 sub_size = 1ULL << nb_qbits;
    
    // Equivalent to total_size / sub_size
    int64 num_blocks = total_size >> nb_qbits; 

    for (int64 b = 0; b < num_blocks; b++) {
        int64 base_idx = spread_bits(b, 0, start_qbit, nb_qbits);
        double complex sum_amps = 0;
        
        // Pass 1: Local Mean
        for (int64 i = 0; i < sub_size; i++) {
            sum_amps += qreg->array[base_idx | (i << start_qbit)];
        }
        
        double complex mean = sum_amps / (double complex)sub_size;

        // Pass 2: Inversion
        for (int64 i = 0; i < sub_size; i++) {
            int64 idx = base_idx | (i << start_qbit);
            qreg->array[idx] = 2.0 * mean - qreg->array[idx];
        }
    }
}

void apply_n_hadamard(QuantumRegister *qreg, int start_qbit, int nb_qbits) {
    for(int i = start_qbit; i < start_qbit + nb_qbits; i++) {
        apply_hadamard(qreg, i);
    }
}

void apply_qft(QuantumRegister *qreg, int start_qbit, int nb_qbits) {
    int64 total_size = qreg->size;
    int64 sub_size = 1ULL << nb_qbits;
    int64 num_blocks = total_size >> nb_qbits;

    double complex *buffer = malloc_custom(sub_size * sizeof(double complex));
    if (!buffer) return;

    for (int64 b = 0; b < num_blocks; b++) {
        int64 base_idx = spread_bits(b, 0, start_qbit, nb_qbits);

        // 1. Extract amplitudes
        for (int64 i = 0; i < sub_size; i++) {
            buffer[i] = qreg->array[base_idx | (i << start_qbit)];
        }

        // 2. Perform QFT
        qft_base(buffer, sub_size, false);

        // 3. Re-insert amplitudes
        for (int64 i = 0; i < sub_size; i++) {
            qreg->array[base_idx | (i << start_qbit)] = buffer[i];
        }
    }

    free_custom(buffer);
}
void apply_iqft(QuantumRegister *qreg, int start_qbit, int nb_qbits) {
    int64 total_size = qreg->size;
    int64 sub_size = (int64)1ULL << nb_qbits;
    int64 num_blocks = total_size >> nb_qbits;

    double complex *buffer = malloc_custom(sub_size * sizeof(double complex));
    if (!buffer) return;

    for (int64 b = 0; b < num_blocks; b++) {
        int64 base_idx = spread_bits(b, 0, start_qbit, nb_qbits);

        // 1. Extract amplitudes
        for (int64 i = 0; i < sub_size; i++) {
            buffer[i] = qreg->array[base_idx | (i << start_qbit)];
        }

        // 2. Perform IQFT
        qft_base(buffer, sub_size, true);

        // 3. Re-insert amplitudes
        for (int64 i = 0; i < sub_size; i++) {
            qreg->array[base_idx | (i << start_qbit)] = buffer[i];
        }
    }

    free_custom(buffer);
}

void apply_prod(QuantumRegister *qreg, int start_qbit, int nb_qbits, int64 a, int64 N) {
    int64 total_size = qreg->size;
    int64 sub_size = 1ULL << nb_qbits;
    int64 num_blocks = total_size >> nb_qbits;

    int64 v = get_inverse(a, N);
    
    double complex *buffer = malloc_custom(sub_size * sizeof(double complex));
    if (!buffer) return;

    for (int64 b = 0; b < num_blocks; b++) {
        int64 base_idx = spread_bits(b, 0, start_qbit, nb_qbits); // Give (b(n-nb)....b(s+1))(0.....0)(bs...b1)

        // Extract amplitudes
        for(int64 i = 0; i < sub_size; i++) {
            buffer[i] = qreg->array[base_idx | (i << start_qbit)];
        }

        // Re-insert amplitudes at correct positions (y[i] = x[iv%N])
        for(int64 i = 0; i < N; i++) {
            qreg->array[base_idx | (i << start_qbit)] = buffer[(v * i)%N];
        }
    }

    free_custom(buffer);
}
void apply_controlled_prod_exp(QuantumRegister *qreg, int c, int start_qbit, int nb_qbits, int64 a, int64 N, int64 k) {
    assert(start_qbit + nb_qbits <= c || c < start_qbit);
    int64 total_size = qreg->size;
    int64 control = 1ULL << c;
    int64 sub_size = 1ULL << nb_qbits;
    int64 num_blocks = total_size >> nb_qbits;

    int64 u = get_inverse(a, N);
    int64 v = fexp_mod(u, k, N);
    
    double complex *buffer = malloc_custom(sub_size * sizeof(double complex));
    if (!buffer) return;

    for (int64 b = 0; b < num_blocks; b++) {
        int64 base_idx = spread_bits(b, 0, start_qbit, nb_qbits); // Give (b(n-nb)....b(s+1))(0.....0)(bs...b1)
        if(!(base_idx & control)) continue;

        // Extract amplitudes
        for(int64 i = 0; i < sub_size; i++) {
            buffer[i] = qreg->array[base_idx | (i << start_qbit)];
        }

        // Re-insert amplitudes at correct positions (y[i] = x[iv%N])
        for(int64 i = 0; i < N; i++) {
            qreg->array[base_idx | (i << start_qbit)] = buffer[(v * i)%N];
        }
    }

    free_custom(buffer);
}

