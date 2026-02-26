#include <complex.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct qregister_s {
    double complex* array;
    uint64_t size;
    int nb_qbits;
} QuantumRegister;

typedef struct cregister_s {
    bool* array;
    int size;
} ClassicalRegister;

// ---- Classical Registers ----
ClassicalRegister *cregister_create(int size);
void cregister_free(ClassicalRegister *creg);

bool cregister_get(ClassicalRegister *creg, int slot);
void cregister_set(ClassicalRegister *creg, int slot, bool value);

uint64_t cregister_calc_number(ClassicalRegister *creg);
void cregister_print(ClassicalRegister *creg);

// ---- Quantum Registers ----
QuantumRegister *qregister_create(int nb_qbits);
void qregister_free(QuantumRegister *qreg);

double complex qregister_get_amplitude(QuantumRegister *qreg, uint64_t index);
void qregister_set_amplitude(QuantumRegister *qreg, uint64_t index, double complex value);

double qregister_calc_norm(QuantumRegister *qreg);
double qregister_calc_proba(QuantumRegister *qreg, int qbit, bool result);
void qregister_normalise(QuantumRegister *qreg);
double qregister_measure(QuantumRegister *qreg, int qbit, ClassicalRegister *creg, int slot);
//bool qregister_split_qbits