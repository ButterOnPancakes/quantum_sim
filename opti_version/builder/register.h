#ifndef REGISTER_H
#define REGISTER_H

#include <complex.h>
#include <stdio.h>

typedef struct {
    int *bits;
    int nb_bits;
} ClassicalRegister;

typedef struct {
    double complex *statevector;
    int nb_qbits;
} QuantumRegister;

ClassicalRegister *cregister_create(int nbits);
void cregister_print(FILE *channel, ClassicalRegister *cregister);
void cregister_free(ClassicalRegister *cregister);

QuantumRegister *qregister_create(int nqubits);
QuantumRegister *qregister_fuse(QuantumRegister *q1, QuantumRegister *q2);
void qregister_print(FILE *channel, QuantumRegister *qregister);
void qregister_free(QuantumRegister *qregister);


#endif