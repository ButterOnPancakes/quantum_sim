#include <complex.h>
#include <stdint.h>
#include <stdbool.h>


#include "../utils/utils.h"

typedef struct qregister_s {
    double complex* array;
    int64 size;
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

int64 cregister_calc_number(ClassicalRegister *creg);
void cregister_print(ClassicalRegister *creg);

// ---- Quantum Registers ----
QuantumRegister *qregister_create(int nb_qbits);
void qregister_free(QuantumRegister *qreg);

double complex qregister_get_amplitude(QuantumRegister *qreg, int64 index);
void qregister_set_amplitude(QuantumRegister *qreg, int64 index, double complex value);
void qregister_set_number(QuantumRegister *qreg, int j);

double qregister_calc_norm(QuantumRegister *qreg);
double qregister_calc_proba(QuantumRegister *qreg, int qbit, bool result);
double qregister_calc_proba_multiple(QuantumRegister *qreg, int number);
void qregister_normalise(QuantumRegister *qreg);
double qregister_measure(QuantumRegister *qreg, int qbit, ClassicalRegister *creg, int slot);
QuantumRegister *qregister_fuse(QuantumRegister *q1, QuantumRegister *q2);

void qregister_print(QuantumRegister *qreg);