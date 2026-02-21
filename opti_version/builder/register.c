#include "register.h"

#include <stdlib.h>
#include <stdint.h>

#include "../../utils/utils.h"

double complex *state_alloc(int nqubits) {
    uint64_t dim = 1ULL << nqubits;
    double complex *s = aligned_alloc_64(dim * sizeof(double complex));
    if (!s) {
        // fallback
        s = malloc(dim * sizeof(double complex));
    }
    return s;
}

ClassicalRegister *cregister_create(int nbits) {
    ClassicalRegister *cregister = malloc(sizeof(ClassicalRegister));
    cregister->nb_bits = nbits;
    cregister->bits = calloc(cregister->nb_bits, sizeof(int));
    return cregister;
}
void cregister_print(FILE *channel, ClassicalRegister *c) {
    fprintf(channel, "[");
    for(int i = 0; i < c->nb_bits; i++) {
        if (i < c->nb_bits - 1) fprintf(channel, "%d, ", c->bits[i]);
        else fprintf(channel, "%d] ", c->bits[i]);
    }

    long int val = 0;
    for(int i = 0; i < c->nb_bits; i++) {
        val <<= 1;
        val += c->bits[i];
    }
    fprintf(channel, "= %ld\n", val);
}
void cregister_free(ClassicalRegister *cregister) {
    free(cregister->bits);
    free(cregister);
}

QuantumRegister *qregister_create(int nqubits) {
    QuantumRegister* qregister = malloc(sizeof(QuantumRegister));
    qregister->nb_qbits = nqubits;
    qregister->statevector = state_alloc(nqubits);

    uint64_t dim = 1ULL << nqubits;
    for (uint64_t i = 0; i < dim; ++i) qregister->statevector[i] = 0.0 + 0.0*I;
    qregister->statevector[0] = 1.0 + 0.0*I;
    return qregister;
}
QuantumRegister *qregister_fuse(QuantumRegister *q1, QuantumRegister *q2) {
    QuantumRegister* qregister = malloc(sizeof(QuantumRegister));
    qregister->nb_qbits = q1->nb_qbits + q2->nb_qbits;
    qregister->statevector = state_alloc(qregister->nb_qbits);

    uint64_t s1 = 1 << q1->nb_qbits;
    uint64_t s2 = 1 << q2->nb_qbits;
    for(uint64_t i = 0; i < s1; i++) {
        for(uint64_t j = 0; j < s2; j++) {
            qregister->statevector[s2 * i + j] = q1->statevector[i] * q2->statevector[j];
        }
    }

    return qregister;
}
void qregister_print(FILE *channel, QuantumRegister *q) {
    fprintf(channel, "[");
    uint64_t dim = 1ULL << q->nb_qbits;
    for(int i = 0; i < dim; i++) {
        if (i < dim - 1) fprintf(channel, "%.2f + i%.2f, ", creal(q->statevector[i]), cimag(q->statevector[i]));
        else fprintf(channel, "%.2f + i%.2f]\n", creal(q->statevector[i]), cimag(q->statevector[i]));
    }
}
void qregister_free(QuantumRegister *qregister) {
    free(qregister->statevector);
    free(qregister);
}
