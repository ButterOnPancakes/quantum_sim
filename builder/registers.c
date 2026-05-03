#include "registers.h"

#include "../utils/utils.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>

// ---- Classical Registers ----
ClassicalRegister *cregister_create(int size) {
    ClassicalRegister *creg = malloc_custom(sizeof(ClassicalRegister));
    creg->array = calloc_custom(size, sizeof(bool));
    creg->size = size;
    return creg;
}
void cregister_free(ClassicalRegister *creg) {
    assert(creg != NULL);
    free_custom(creg->array);
    free_custom(creg);
}

bool cregister_get(ClassicalRegister *creg, int slot) {
    assert(creg != NULL && slot < creg->size);
    return creg->array[slot];
}
void cregister_set(ClassicalRegister *creg, int slot, bool value) {
    assert(creg != NULL && slot < creg->size);
    creg->array[slot] = value;
}

int64 cregister_calc_number(ClassicalRegister *creg) {
    int64 number = 0;
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
    int64 dim = 1ULL << nb_qbits;
    QuantumRegister *qreg = malloc_custom(sizeof(QuantumRegister));
    qreg->array = calloc_custom(dim, sizeof(double complex));
    assert(qreg->array != NULL);
    qreg->array[0] = 1.0;
    qreg->size = dim;
    qreg->nb_qbits = nb_qbits;
    return qreg;
}
void qregister_free(QuantumRegister *qreg) {
    assert(qreg != NULL);
    free_custom(qreg->array);
    free_custom(qreg);
}

void qregister_print(QuantumRegister *qreg) {
    if(qreg->nb_qbits > 4) return;
    for(int64 i = 0; i < qreg->size; i++) {
        printf("%f + i%f\n", creal(qreg->array[i]), cimag(qreg->array[i]));
    }
}

double complex qregister_get_amplitude(QuantumRegister *qreg, int64 index) {
    assert(qreg != NULL && index < qreg->size);
    return qreg->array[index];
}
void qregister_set_amplitude(QuantumRegister *qreg, int64 index, double complex value) {
    assert(qreg != NULL && index < qreg->size);
    qreg->array[index] = value;
}
void qregister_set_number(QuantumRegister *qreg, int j) {
    for(int64 i = 0; i < qreg->size; i++) {
        qreg->array[i] = 0;
    }
    qreg->array[j] = 1;
}

double qregister_calc_norm(QuantumRegister *qreg) {
    double norm = 0;
    for(int64 i = 0; i < qreg->size; i++) {
        norm += cabs(qreg->array[i]) * cabs(qreg->array[i]);
    }
    return sqrt(norm);
}
double qregister_calc_proba(QuantumRegister *qreg, int qbit, bool result) {
    int64 size = qreg->size;
    int64 bit_weight = 1ULL << qbit;
    double proba = 0;
    for(int64 i = 0; i < size; i++) {
        bool bit_val = (i & bit_weight) != 0;
        if(bit_val == result) {
            proba += cabs(qreg->array[i]) * cabs(qreg->array[i]);
        }
    }
    return proba;
}
double qregister_calc_proba_multiple(QuantumRegister *qreg, int number) {
    double proba = 1.;
    for(int i = 0; i < qreg->nb_qbits; i++) {
        proba *= qregister_calc_proba(qreg, i, number & (1 << i));
    }
    return proba;
}
void qregister_normalise(QuantumRegister *qreg) {
    double norm = qregister_calc_norm(qreg);
    if (norm < EPSILON) return;
    for(int64 i = 0; i < qreg->size; i++) {
        qreg->array[i] = qreg->array[i] / norm;
    }
}
void qregister_collapse(QuantumRegister *qreg, int qbit, bool result) {
    int64 size = qreg->size;
    int64 bit_weight = 1ULL << qbit;
    for(int64 i = 0; i < size; i++) {
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
    bool result = random > proba0;
    double proba_measured = result ? 1.0 - proba0 : proba0;
    //printf("Measured %b with probability %f\n", result, proba_measured);
    qregister_collapse(qreg, qbit, result);

    if(creg != NULL) cregister_set(creg, slot, result);
    return proba_measured;
}

QuantumRegister *qregister_fuse(QuantumRegister *q1, QuantumRegister *q2) {
    QuantumRegister* qregister = malloc_custom(sizeof(QuantumRegister));
    qregister->nb_qbits = q1->nb_qbits + q2->nb_qbits;
    qregister->size = 1ULL << qregister->nb_qbits;
    qregister->array = malloc_custom(qregister->size * sizeof(double complex));

    int64 s1 = 1ULL << q1->nb_qbits;
    int64 s2 = 1ULL << q2->nb_qbits;
    for(int64 j = 0; j < s2; j++) {
        for(int64 i = 0; i < s1; i++) {
            qregister->array[s1 * j + i] = q1->array[i] * q2->array[j];
        }
    }

    return qregister;
}


