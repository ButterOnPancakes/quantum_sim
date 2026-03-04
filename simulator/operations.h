#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "../builder/registers.h"

/* 
 * 1-qbit gates
 * Transformations on a single qubit 'qbit'.
 */

/**
 * @brief Applies the Hadamard gate (H) to a qubit.
 * Matrix: 1/sqrt(2) * [1  1]
 *                     [1 -1]
 */
void apply_hadamard(QuantumRegister *qreg, int qbit);

/**
 * @brief Applies the Pauli-X gate (NOT gate) to a qubit.
 * Matrix: [0 1]
 *         [1 0]
 */
void apply_gate_x(QuantumRegister *qreg, int qbit);

/**
 * @brief Applies the Pauli-Y gate to a qubit.
 * Matrix: [0 -i]
 *         [i  0]
 */
void apply_gate_y(QuantumRegister *qreg, int qbit);

/**
 * @brief Applies the Pauli-Z gate to a qubit.
 * Matrix: [1  0]
 *         [0 -1]
 */
void apply_gate_z(QuantumRegister *qreg, int qbit);

/**
 * @brief Applies a phase shift gate (Rz) to a qubit.
 * Matrix: [1       0      ]
 *         [0  exp(i*phase)]
 */
void apply_gate_phase(QuantumRegister *qreg, int qbit, double phase);

/* 
 * 2-qbit gates
 * Transformations involving two qubits.
 */

/**
 * @brief Applies the Controlled-NOT (CNOT) gate.
 * Flips the 'target' qubit if the 'control' qubit is 1.
 */
void apply_cnot(QuantumRegister *qreg, int control, int target);

/**
 * @brief Applies the SWAP gate.
 * Exchanges the states of 'qbit1' and 'qbit2'.
 */
void apply_swap(QuantumRegister *qreg, int qbit1, int qbit2);

/* 
 * n-qbit gates
 * Transformations involving multiple qubits or specific sub-registers.
 */

/**
 * @brief Applies a phase oracle.
 * Multiplies the amplitude of states satisfying 'function' by -1.
 * @param start_qbit Index of the first qubit in the input register.
 * @param nb_qbits Number of qubits in the input register.
 * @param function Predicate function for marking states.
 */
void apply_oracle_phase(QuantumRegister *qreg, int start_qbit, int nb_qbits, bool (*function)(uint64_t number));

/**
 * @brief Applies an ancilla-based oracle.
 * Flips the 'ancilla_qbit' if the input register [start, start+nb] satisfies 'function'.
 */
void apply_oracle_ancilla(QuantumRegister *qreg, int start_qbit, int nb_qbits, int ancilla_qbit, bool (*function)(uint64_t number));

/**
 * @brief Applies the Grover diffusion operator (inversion about the mean).
 * Acts on a sub-register starting at 'start_qbit' of length 'nb_qbits'.
 */
void apply_diffusion(QuantumRegister *qreg, int start_qbit, int nb_qbits);

/**
 * @brief Applies a Hadamard gate to 'nb_qbits' contiguous qubits.
 */
void apply_n_hadamard(QuantumRegister *qreg, int start_qbit, int nb_qbits);

/**
 * @brief Applies the Quantum Fourier Transform (QFT).
 * Transformation: |j> -> 1/sqrt(N) * sum_{k=0}^{N-1} exp(2*pi*i*j*k/N) |k>
 */
void apply_qft(QuantumRegister *qreg, int start_qbit, int nb_qbits);

/**
 * @brief Applies the Inverse Quantum Fourier Transform (IQFT).
 */
void apply_iqft(QuantumRegister *qreg, int start_qbit, int nb_qbits);

#endif // OPERATIONS_H
