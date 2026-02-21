#include "operations.h"
#include "../builder/registers.h"

#include <complex.h>

bool apply_measure(QuantumRegister *qreg, int qbit, ClassicalRegister *creg, int slot);

// 1-qbit gate
void apply_hadamard(QuantumRegister *qreg, int qbit);
void apply_gate_x(QuantumRegister *qreg, int qbit);
void apply_gate_y(QuantumRegister *qreg, int qbit);
void apply_gate_z(QuantumRegister *qreg, int qbit);
void apply_gate_phase(QuantumRegister *qreg, int qbit, double phase);

// 2-qbit gates
void apply_cnot(QuantumRegister *qreg, int control, int target);
void apply_swap(QuantumRegister *qreg, int qbit1, int qbit2);

// n-qbit gates
void apply_n_hadamard(QuantumRegister *qreg, int start_qbit, int nb_qbits);

void apply_qft(QuantumRegister *qreg, int start_qbit, int nb_qbits);
void apply_oracle_ancilla(QuantumRegister *qreg, int start_qbit, int nb_qbits, int ancilla_qbit, bool (*function)(int number));
void apply_oracle_phase(QuantumRegister *qreg, int start_qbit, int nb_qbits, bool (*function)(int number));
