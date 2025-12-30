#include "circuit.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#include "../utils/list.h"
#include "../utils/utils.h"

QuantumCircuit *circuit_create(QuantumRegister *qregister, ClassicalRegister *cregister) {
    QuantumCircuit *circuit = malloc(sizeof(QuantumCircuit));
    circuit->gates = list_create();
    circuit->qregister = qregister;
    circuit->cregister = cregister;
    return circuit;
}
void circuit_free(QuantumCircuit *circuit) {
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

void circuit_print(FILE *channel, QuantumCircuit *circuit) {
    for(int i = 0; i < circuit->qregister->nb_qbits; i++) {
        fprintf(channel, "q%.2d: ", i);

        ListIterator iter = list_iterator_begin(circuit->gates);
        while (list_iterator_has_next(&iter)) {
            Gate *gate = list_iterator_next(&iter);

            switch(gate->class) {
                case MEAS:
                    if(gate->gate.measure.qbit == i) fprintf(channel, "-|M(%.2d)|-", gate->gate.measure.cbit);
                    else fprintf(channel, "---------");
                    continue;
                case CUSTOM: 
                    bool found = false;
                    for(int j = 0; j < gate->gate.custom.nb_qbits; j++) {
                        if(gate->gate.custom.qbits[j] == i) {
                            fprintf(channel, "-|%-5.5s|-", gate->gate.custom.label);
                            found = true;
                            break;
                        }
                    }
                    if(!found) fprintf(channel, "---------");
                    continue;
                case UNITARY: 
                    if(gate->gate.unitary.qbit == i) fprintf(channel, get_symbol(gate->gate.unitary.type));
                    else fprintf(channel, "---------");
                    continue;
                case CONTROL: 
                    if(gate->gate.control.qbit == i) fprintf(channel, get_symbol(gate->gate.control.type));
                    else if(gate->gate.control.control == i) fprintf(channel, "-|  *  |-");
                    else fprintf(channel, "---------");
                    continue;
            }
        }
        fprintf(channel, "\n");
    }
}

void add_unitary_gate(QuantumCircuit *circuit, int t, SingleBitGate tg, double phase) {
    list_append(circuit->gates, create_unitary_gate(t, tg, phase));
}
void add_control_gate(QuantumCircuit *circuit, int c, int t, SingleBitGate tg, double phase) {
    list_append(circuit->gates, create_control_gate(c, t, tg, phase));
}
// Mat size must be 2^nb_qbits !
void add_custom_gate(QuantumCircuit *circuit, int nb_qbits, int *t, double complex *mat, char *label) {
    Gate *gate = create_custom_gate(nb_qbits, t, mat, label);
    list_append(circuit->gates, gate);
}
void add_measure(QuantumCircuit *circuit, int qbit, int cbit) {
    Gate *gate = create_measure(qbit, cbit);
    list_append(circuit->gates, gate);
}