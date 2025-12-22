#include "circuit.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#include "../../utils/list.h"
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

ClassicalRegister *create_cregister(int nbits) {
    ClassicalRegister *cregister = malloc(sizeof(ClassicalRegister));
    cregister->nb_bits = nbits;
    cregister->bits = calloc(cregister->nb_bits, sizeof(int));
    return cregister;
}
void print_cregister(ClassicalRegister *c) {
    printf("[");
    for(int i = 0; i < c->nb_bits; i++) {
        if (i < c->nb_bits - 1) printf("%d, ", c->bits[i]);
        else printf("%d] ", c->bits[i]);
    }

    long int val = 0;
    for(int i = 0; i < c->nb_bits; i++) {
        val <<= 1;
        val += c->bits[i];
    }
    printf("= %ld\n", val);
}
void free_cregister(ClassicalRegister *cregister) {
    free(cregister->bits);
    free(cregister);
}

QuantumRegister *create_qregister(int nqubits) {
    QuantumRegister* qregister = malloc(sizeof(QuantumRegister));
    qregister->nb_qbits = nqubits;
    qregister->statevector = state_alloc(nqubits);

    uint64_t dim = 1ULL << nqubits;
    for (uint64_t i = 0; i < dim; ++i) qregister->statevector[i] = 0.0 + 0.0*I;
    qregister->statevector[0] = 1.0 + 0.0*I;
    return qregister;
}
QuantumRegister *fuse_qregister(QuantumRegister *q1, QuantumRegister *q2) {
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
void print_qregister(QuantumRegister *q) {
    printf("[");
    for(int i = 0; i < q->nb_qbits; i++) {
        if (i < q->nb_qbits - 1) printf("%.2f + i%.2f, ", creal(q->statevector[i]), cimag(q->statevector[i]));
        else printf("%.2f + i%.2f]\n", creal(q->statevector[i]), cimag(q->statevector[i]));
    }
}
void free_qregister(QuantumRegister *qregister) {
    free(qregister->statevector);
    free(qregister);
}

QuantumCircuit *create_circuit(QuantumRegister *qregister, ClassicalRegister *cregister) {
    QuantumCircuit *circuit = malloc(sizeof(QuantumCircuit));
    circuit->gates = list_create();
    circuit->qregister = qregister;
    circuit->cregister = cregister;
    return circuit;
}
void destroy_circuit(QuantumCircuit *circuit) {
    ListIterator iter = list_iterator_begin(circuit->gates);
    while (list_iterator_has_next(&iter)) {
        Gate *gate = list_iterator_next(&iter);
        free(gate);
    }
    list_destroy(circuit->gates);
    free(circuit);
}

char *get_symbol(SingleBitGate gt) {
    switch(gt) {
        case GATE_I: return "-|  I  |-";
        case GATE_H: return "-|  H  |-";
        case GATE_X: return "-|  X  |-";
        case GATE_Y: return "-|  Y  |-";
        case GATE_Z: return "-|  Z  |-";
    }
    return "-|?????|-";
}

void print_circuit(QuantumCircuit *circuit) {
    for(int i = 0; i < circuit->qregister->nb_qbits; i++) {
        printf("q%.2d: ", i);

        ListIterator iter = list_iterator_begin(circuit->gates);
        while (list_iterator_has_next(&iter)) {
            Gate *gate = list_iterator_next(&iter);

            switch(gate->class) {
                case MEAS:
                    if(gate->gate.measure.qbit == i) printf("-|M(%.2d)|-", gate->gate.measure.cbit);
                    else printf("---------");
                    continue;
                case CUSTOM: 
                    bool found = false;
                    for(int j = 0; j < gate->gate.custom.nb_qbits; j++) {
                        if(gate->gate.custom.qbits[j] == i) {
                            printf("-|%-5.5s|-", gate->gate.custom.label);
                            found = true;
                            break;
                        }
                    }
                    if(!found) printf("---------");
                    continue;
                case UNITARY: 
                    if(gate->gate.unitary.qbit == i) printf(get_symbol(gate->gate.unitary.type));
                    else printf("---------");
                    continue;
                case CONTROL: 
                    if(gate->gate.control.qbit == i) printf(get_symbol(gate->gate.control.type));
                    else if(gate->gate.control.control == i) printf("-|  *  |-");
                    else printf("---------");
                    continue;
            }
        }
        printf("\n");
    }
}

void add_unitary_gate(QuantumCircuit *circuit, int t, SingleBitGate tg) {
    Gate *gate = malloc(sizeof(Gate));
    gate->class = UNITARY;
    gate->gate.unitary.type = tg;
    gate->gate.unitary.qbit = t;
    list_append(circuit->gates, gate);
}
void add_control_gate(QuantumCircuit *circuit, int c, int t, SingleBitGate tg) {
    Gate *gate = malloc(sizeof(Gate));
    gate->class = CONTROL;
    gate->gate.control.control = c;
    gate->gate.control.qbit = t;
    gate->gate.control.type = tg;
    list_append(circuit->gates, gate);
}
// Mat size must be 2^nb_qbits !
void add_custom_gate(QuantumCircuit *circuit, int nb_qbits, int *t, double complex *mat, char *label) {
    Gate *gate = malloc(sizeof(Gate));
    gate->class = CUSTOM;
    gate->gate.custom.nb_qbits = nb_qbits;
    gate->gate.custom.qbits = t;
    gate->gate.custom.mat = mat;
    gate->gate.custom.label = label;
    list_append(circuit->gates, gate);
}
void add_measure(QuantumCircuit *circuit, int qbit, int cbit) {
    Gate *gate = malloc(sizeof(Gate));
    gate->class = MEAS;
    gate->gate.measure.cbit = cbit;
    gate->gate.measure.qbit = qbit;
    list_append(circuit->gates, gate);
}