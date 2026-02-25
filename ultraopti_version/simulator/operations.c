#include "operations.h"
#include "../builder/registers.h"
#include "../../utils/utils.h"
#include "../../utils/fft.h"

#include <complex.h>
#include <stdlib.h>
#include <math.h>

// 1-qbit gate, issu de opti_version
void apply_hadamard(QuantumRegister *qreg, int qbit) {
    uint64_t size = 1 << qreg->nb_qbits;
    uint64_t bit = 1 << qbit; // Little Endian: bit 0 is the 2^0 position
    
    double inv_sqrt2 = sqrt(0.5);

    for (uint64_t i = 0; i < size; i += (bit << 1)) {
        for (uint64_t r = 0; r < bit; r++) {
            uint64_t i0 = i + r;
            uint64_t i1 = i0 + bit;
            
            double complex a0 = qreg->array[i0];
            double complex a1 = qreg->array[i1];
            
            qreg->array[i0] = (a0 + a1) * inv_sqrt2;
            qreg->array[i1] = (a0 - a1) * inv_sqrt2;
        }
    }
}
void apply_gate_x(QuantumRegister *qreg, int qbit) {
    uint64_t size = 1 << qreg->nb_qbits;
    uint64_t bit = 1 << qbit;
    
    for (uint64_t i = 0; i < size; i += (bit << 1)) {
        for (uint64_t r = 0; r < bit; r++) {
            uint64_t i0 = i + r;
            uint64_t i1 = i0 + bit;
            double complex tmp = qreg->array[i0];
            qreg->array[i0] = qreg->array[i1];
            qreg->array[i1] = tmp;
        }
    }
}
void apply_gate_y(QuantumRegister *qreg, int qbit) {
    uint64_t size = 1 << qreg->nb_qbits;
    uint64_t bit = 1 << qbit;
    
    for (uint64_t i = 0; i < size; i += (bit << 1)) {
        for (uint64_t r = 0; r < bit; r++) {
            uint64_t i0 = i + r;
            uint64_t i1 = i0 + bit;
            double complex a0 = qreg->array[i0];
            double complex a1 = qreg->array[i1];
            qreg->array[i0] = -I * a1; 
            qreg->array[i1] =  I * a0;
        }
    }
}
void apply_gate_z(QuantumRegister *qreg, int qbit) {
    uint64_t size = 1 << qreg->nb_qbits;
    uint64_t bit = 1 << qbit;
    
    for (uint64_t i = 0; i < size; i += (bit << 1)) {
        for (uint64_t r = 0; r < bit; r++) {
            qreg->array[i + r + bit] *= -1.0;
        }
    }
}
void apply_gate_phase(QuantumRegister *qreg, int qbit, double phase) {
    uint64_t size = 1 << qreg->nb_qbits;
    uint64_t bit = 1 << qbit;
    double complex expo = cexp(I * phase);
    
    for (uint64_t i = 0; i < size; i += (bit << 1)) {
        for (uint64_t r = 0; r < bit; r++) {
            qreg->array[i + r + bit] *= expo;
        }
    }
}

// 2-qbit gates, issu de opti_version
void apply_cnot(QuantumRegister *qreg, int c, int t) {
    uint64_t size = 1 << qreg->nb_qbits;
    uint64_t c_bit = 1 << c;
    uint64_t t_bit = 1 << t;

    // We only need to iterate through half the register (where control bit is 1)
    for (uint64_t i = 0; i < size; i++) {
        if ((i & c_bit) && !(i & t_bit)) {
            uint64_t i0 = i;
            uint64_t i1 = i | t_bit;
            double complex tmp = qreg->array[i0];
            qreg->array[i0] = qreg->array[i1];
            qreg->array[i1] = tmp;
        }
    }
}
void apply_swap(QuantumRegister *qreg, int q0, int q1) {
    uint64_t size = 1 << qreg->nb_qbits;
    uint64_t b0 = 1 << q0;
    uint64_t b1 = 1 << q1;

    for (uint64_t i = 0; i < size; i++) {
        // Only swap if bits at q0 and q1 are different (01 or 10)
        // We act when bit q0=1 and q1=0 to perform the swap once
        if ((i & b0) && !(i & b1)) {
            uint64_t i_01 = (i & ~b0) | b1; // This is the state where bits are swapped
            double complex tmp = qreg->array[i];
            qreg->array[i] = qreg->array[i_01];
            qreg->array[i_01] = tmp;
        }
    }
}

// n-qbit gates
void apply_oracle_phase(QuantumRegister *qreg, int start_qbit, int nb_qbits, bool (*function)(int number)) {
    uint64_t size = qreg->size;
    uint64_t input_mask = (1 << nb_qbits) - 1;

    for (uint64_t i = 0; i < size; i++) {
        int number = (int)((i >> start_qbit) & input_mask);

        if (function(number)) {
            qreg->array[i] = -qreg->array[i];
        }
    }
}
void apply_oracle_ancilla(QuantumRegister *qreg, int start_qbit, int nb_qbits, int ancilla_qbit, bool (*function)(int number)) {
    uint64_t size = 1 << qreg->nb_qbits;
    uint64_t ancilla_bit = 1 << ancilla_qbit; // Little Endian
    uint64_t input_mask = (1 << nb_qbits) - 1;

    //#pragma omp parallel for
    for (uint64_t i = 0; i < size; i++) {
        // We only process the pair when the ancilla bit is 0
        // This ensures we only perform the swap once per pair
        if (!(i & ancilla_bit)) {
            int number = (int)((i >> start_qbit) & input_mask);

            if (function(number)) {
                uint64_t i0 = i;
                uint64_t i1 = i | ancilla_bit;

                double complex temp = qreg->array[i0];
                qreg->array[i0] = qreg->array[i1];
                qreg->array[i1] = temp;
            }
        }
    }
}
void apply_diffusion(QuantumRegister *qreg, int start_qbit, int nb_qbits) {
    uint64_t total_size = qreg->size;
    uint64_t sub_size = 1 << nb_qbits;
    
    // Equivalent to total_size / sub_size
    uint64_t num_blocks = total_size >> nb_qbits; 

    for (uint64_t b = 0; b < num_blocks; b++) {
        // Construct the base index by spreading bits of 'b'
        uint64_t low_bits = b & ((1 << start_qbit) - 1);
        uint64_t high_bits = (b >> start_qbit) << (start_qbit + nb_qbits);
        uint64_t base_idx = high_bits | low_bits;

        double complex sum_amps = 0;
        
        // Pass 1: Local Mean
        for (uint64_t i = 0; i < sub_size; i++) {
            sum_amps += qreg->array[base_idx | (i << start_qbit)];
        }
        
        double complex mean = sum_amps / (double complex)sub_size;

        // Pass 2: Inversion
        for (uint64_t i = 0; i < sub_size; i++) {
            uint64_t idx = base_idx | (i << start_qbit);
            qreg->array[idx] = 2.0 * mean - qreg->array[idx];
        }
    }
}

void apply_n_hadamard(QuantumRegister *qreg, int start_qbit, int nb_qbits) {
    for(int i = start_qbit; i < start_qbit + nb_qbits; i++) {
        apply_hadamard(qreg, i);
    }
}

// Helper to reverse bits within a specific window for Little Endian
uint64_t reverse_window_bits(uint64_t val, int bits) {
    uint64_t res = 0;
    for (int i = 0; i < bits; i++) {
        if (val & (1ULL << i)) {
            res |= (1ULL << (bits - 1 - i));
        }
    }
    return res;
}
void apply_qft(QuantumRegister *qreg, int start_qbit, int nb_qbits) {
    uint64_t total_size = qreg->size;
    uint64_t sub_size = 1 << nb_qbits;
    uint64_t num_blocks = total_size >> nb_qbits;

    double complex *buffer = malloc(sub_size * sizeof(double complex));
    if (!buffer) return;

    for (uint64_t b = 0; b < num_blocks; b++) {
        // Construct the base index by spreading bits of 'b' around the nb_qbits window
        uint64_t low_bits = b & ((1 << start_qbit) - 1);
        uint64_t high_bits = (b >> start_qbit) << (start_qbit + nb_qbits);
        uint64_t base_idx = high_bits | low_bits;

        // 1. Extract amplitudes
        for (uint64_t i = 0; i < sub_size; i++) {
            buffer[i] = qreg->array[base_idx | (i << start_qbit)];
        }

        // 2. Perform QFT
        qft_base(buffer, sub_size, false);

        // 3. Re-insert amplitudes
        for (uint64_t i = 0; i < sub_size; i++) {
            qreg->array[base_idx | (i << start_qbit)] = buffer[i];
        }
    }

    free(buffer);
}

void apply_iqft(QuantumRegister *qreg, int start_qbit, int nb_qbits) {
    uint64_t total_size = qreg->size;
    uint64_t sub_size = (uint64_t)1 << nb_qbits;
    uint64_t num_blocks = total_size >> nb_qbits;

    double complex *buffer = malloc(sub_size * sizeof(double complex));
    if (!buffer) return;

    for (uint64_t b = 0; b < num_blocks; b++) {
        uint64_t low_bits = b & (((uint64_t)1 << start_qbit) - 1);
        uint64_t high_bits = (b >> start_qbit) << (start_qbit + nb_qbits);
        uint64_t base_idx = high_bits | low_bits;

        for (uint64_t i = 0; i < sub_size; i++) {
            buffer[i] = qreg->array[base_idx | (i << start_qbit)];
        }

        qft_base(buffer, sub_size, true);

        for (uint64_t i = 0; i < sub_size; i++) {
            qreg->array[base_idx | (i << start_qbit)] = buffer[i];
        }
    }

    free(buffer);
}