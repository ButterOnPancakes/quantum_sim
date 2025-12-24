#include "circuit_old.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>

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
double complex* init_zero_state(int nqubits) {
    double complex *s = state_alloc(nqubits);
    uint64_t dim = 1ULL << nqubits;
    for (uint64_t i = 0; i < dim; ++i) s[i] = 0.0 + 0.0*I;
    s[0] = 1.0 + 0.0*I;
    return s;
}

double complex* fuse_qbits(double complex *q1, int n1, double complex *q2, int n2) {
    uint64_t s1 = 1 << n1;
    uint64_t s2 = 1 << n2;
    double complex *result = state_alloc(n1 + n2);
    for(uint64_t i = 0; i < s1; i++) {
        for(uint64_t j = 0; j < s2; j++) {
            result[s2 * i + j] = q1[i] * q2[j];
        }
    }

    return result;
}

QuantumCircuit *circuit_create(int n_qbits) {
    QuantumCircuit *circuit = malloc(sizeof(QuantumCircuit));
    circuit->nb_qbits = n_qbits;
    circuit->gates = list_create();
    return circuit;
}
void circuit_free(QuantumCircuit *circuit) {
    ListIterator iter = list_iterator_begin(circuit->gates);
    while (list_iterator_has_next(&iter)) {
        Gate *gate = list_iterator_next(&iter);
        free(gate);
    }
    list_destroy(circuit->gates);
}

void circuit_print(QuantumCircuit *circuit) {
    for(int i = 0; i < circuit->nb_qbits; i++) {
        printf("q%.2d: ", i);

        ListIterator iter = list_iterator_begin(circuit->gates);
        while (list_iterator_has_next(&iter)) {
            Gate *gate = list_iterator_next(&iter);
            if(gate->type == CNOT && gate->params[1] == i) {
                printf("-|  o  |-");
                continue;
            }
            if(gate->params[0] <= i && i < gate->params[0] + gate->params[1]) {
                if(gate->type == ORACLE) {
                    printf("-| ORA |-");
                    continue;
                }
                if(gate->type == S0) {
                    printf("-|  S  |-");
                    continue;
                }
            }

            if(i != gate->params[0]) {
                printf("---------");
                continue;
            }

            switch(gate->type) {
                case ID: printf("-|  I  |-"); break;
                case X: printf("-|  X  |-"); break;
                case Z: printf("-|  Z  |-"); break;
                case H: printf("-|  H  |-"); break;
                case MEAS: printf("-|M(%.2d)|-", gate->params[1]); break;
                case CNOT: printf("-|  +  |-"); break;
                case ORACLE: printf("-| ORA |-"); break;
                case S0: printf("-|  S  |-"); break;
                default: printf("-|?????|-"); break;
            }
        }
        printf("\n");
    }
}

void add_single_qbit_gate(QuantumCircuit *circuit, int row, GateType g) {
    assert(g != MEAS);
    Gate *gate = malloc(sizeof(Gate));
    gate->type = g;
    gate->params[0] = row; gate->params[1] = -1;
    list_append(circuit->gates, gate);
}
void add_multiple_qbit_gate(QuantumCircuit *circuit, int row, int nb_qbits, GateType g, bool (*f)(int *t, int n)) {
    Gate *gate = malloc(sizeof(Gate));
    gate->type = g;
    gate->params[0] = row; gate->params[1] = nb_qbits;
    gate->f = f;
    list_append(circuit->gates, gate);
}
void add_double_qbit_gate(QuantumCircuit *circuit, int row, int control, GateType g) {
    assert(g == CNOT);
    Gate *gate = malloc(sizeof(Gate));
    gate->type = g;
    gate->params[0] = row; gate->params[1] = control;
    list_append(circuit->gates, gate);
}
void add_single_qbit_measure(QuantumCircuit *circuit, int row, int output) {
    Gate *gate = malloc(sizeof(Gate));
    gate->type = MEAS;
    gate->params[0] = row; gate->params[1] = output;
    list_append(circuit->gates, gate);
}
