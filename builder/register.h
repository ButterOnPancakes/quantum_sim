#ifndef REGISTER_H
#define REGISTER_H

#include <complex.h>
#include <stdio.h>

typedef struct ClassicalRegister ClassicalRegister;
typedef struct QuantumRegister QuantumRegister;

int cregister_get_num_bits(const ClassicalRegister *cregister);
int cregister_get_bit(const ClassicalRegister *cregister, int index);

int qregister_get_num_qubits(const QuantumRegister *qregister);
double complex *qregister_get_statevector(const QuantumRegister *qregister);

ClassicalRegister *cregister_create(int nbits);
void cregister_print(FILE *channel, ClassicalRegister *cregister);
void cregister_free(ClassicalRegister *cregister);

QuantumRegister *qregister_create(int nqubits);
QuantumRegister *qregister_fuse(QuantumRegister *q1, QuantumRegister *q2);
void qregister_print(FILE *channel, QuantumRegister *qregister);
void qregister_free(QuantumRegister *qregister);


#endif