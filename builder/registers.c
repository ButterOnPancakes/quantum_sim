#include "registers.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>

// ---- Classical Registers ----
ClassicalRegister *cregister_create(int size) {
    ClassicalRegister *creg = malloc(sizeof(ClassicalRegister));
    creg->array = calloc(size, sizeof(bool));
    creg->size = size;
    return creg;
}
void cregister_free(ClassicalRegister *creg) {
    assert(creg != NULL);
    free(creg->array);
    free(creg);
}

bool cregister_get(ClassicalRegister *creg, int slot) {
    assert(creg != NULL && slot < creg->size);
    return creg->array[slot];
}
void cregister_set(ClassicalRegister *creg, int slot, bool value) {
    assert(creg != NULL && slot < creg->size);
    creg->array[slot] = value;
}

uint64_t cregister_calc_number(ClassicalRegister *creg) {
    uint64_t number = 0;
    for(int i = 0; i < creg->size; i++) {
        if(creg->array[i]) {
            number |= (1ULL << i);
        }
    }
    return number;
}
void cregister_print(ClassicalRegister *creg) {
    for(int i = 0; i < creg->size; i++) {
        printf("%d ", creg->array[i]);
    }
    printf("\n");
}

// ---- Quantum Registers ----
QuantumRegister *qregister_create(int nb_qbits) {
    uint64_t dim = 1ULL << nb_qbits;
    QuantumRegister *qreg = malloc(sizeof(QuantumRegister));
    qreg->array = calloc(dim, sizeof(double complex));
    assert(qreg->array != NULL);
    qreg->array[0] = 1.0;
    qreg->size = dim;
    qreg->nb_qbits = nb_qbits;
    return qreg;
}
void qregister_free(QuantumRegister *qreg) {
    assert(qreg != NULL);
    free(qreg->array);
    free(qreg);
}

double complex qregister_get_amplitude(QuantumRegister *qreg, uint64_t index) {
    assert(qreg != NULL && index < qreg->size);
    return qreg->array[index];
}
void qregister_set_amplitude(QuantumRegister *qreg, uint64_t index, double complex value) {
    assert(qreg != NULL && index < qreg->size);
    qreg->array[index] = value;
}

double qregister_calc_norm(QuantumRegister *qreg) {
    double norm = 0;
    for(uint64_t i = 0; i < qreg->size; i++) {
        norm += cabs(qreg->array[i]) * cabs(qreg->array[i]);
    }
    return sqrt(norm);
}
double qregister_calc_proba(QuantumRegister *qreg, int qbit, bool result) {
    uint64_t size = qreg->size;
    uint64_t bit_weight = 1ULL << qbit;
    double proba = 0;
    for(uint64_t i = 0; i < size; i++) {
        bool bit_val = (i & bit_weight) != 0;
        if(bit_val == result) {
            proba += cabs(qreg->array[i]) * cabs(qreg->array[i]);
        }
    }
    return proba;
}
void qregister_normalise(QuantumRegister *qreg) {
    double norm = qregister_calc_norm(qreg);
    if (norm < 1e-15) return;
    for(uint64_t i = 0; i < qreg->size; i++) {
        qreg->array[i] = qreg->array[i] / norm;
    }
}
void qregister_collapse(QuantumRegister *qreg, int qbit, bool result) {
    uint64_t size = qreg->size;
    uint64_t bit_weight = 1ULL << qbit;
    for(uint64_t i = 0; i < size; i++) {
        bool bit_val = (i & bit_weight) != 0;
        if(bit_val != result) {
            qreg->array[i] = 0.0;
        }
    }
    qregister_normalise(qreg);
}
double qregister_measure(QuantumRegister *qreg, int qbit, ClassicalRegister *creg, int slot) {
    double proba0 = qregister_calc_proba(qreg, qbit, 0);
    double random = (double)rand() / (double)RAND_MAX;
    bool result = random >= proba0;
    qregister_collapse(qreg, qbit, result);

    if(creg != NULL) cregister_set(creg, slot, result);
    return result ? 1.0 - proba0 : proba0;
}