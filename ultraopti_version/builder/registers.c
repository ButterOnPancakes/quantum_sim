#include "registers.h"

// ---- Classical Registers ----
ClassicalRegister *cregister_create(int size);
void cregister_free(ClassicalRegister *creg);

bool cregister_get(ClassicalRegister *creg, int slot);
void cregister_set(ClassicalRegister *creg, int slot, bool value);

// ---- Quantum Registers ----
QuantumRegister *qregister_create(int nb_qbits);
void qregister_free(QuantumRegister *qreg);

double complex qregister_get_amplitude(QuantumRegister *qreg, uint64_t index);
void qregister_set_amplitude(QuantumRegister *qreg, uint64_t index);

void qregister_normalise(QuantumRegister *qreg);