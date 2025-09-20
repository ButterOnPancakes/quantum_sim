#include "circuit.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "../utils/dynarrays/dynarray.h"

typedef struct {
    GateType gt;
    int params[];
} Gate;

typedef struct {
    enum { GATE, MEASURE } type;
    union {
        Gate gate;
        struct {
            int qbit;
            int bit;
        } measure;
    } operation;
} Instruction;

struct qcircuit {
    int n_qbits;
    DynArray *instructions;
};

QuantumCircuit *create_circuit(int n_qbits) {
    QuantumCircuit *circuit = malloc(sizeof(QuantumCircuit));
    circuit->n_qbits = n_qbits;
    circuit->instructions = dynarray_create(POINTER_TYPE, 20);
    return circuit;
}
int destroy_circuit(QuantumCircuit *circuit) {
    int size = dynarray_len(circuit->instructions);
    for(int i = 0; i < size; i++) {
        Instruction *instr = dynarray_get(circuit->instructions, i).pointer_val;
        free(instr);
    }
    dynarray_free(circuit->instructions);
    return circuit->instructions == NULL ? 1 : 0;
}

void print_circuit(QuantumCircuit *circuit) {
    int size = dynarray_len(circuit->instructions);
    for(int qbit = 0; qbit < circuit->n_qbits; qbit++) {
        printf("q%.2d: ---", qbit);
        for(int i = 0; i < size; i++) {
            Instruction *instr = dynarray_get(circuit->instructions, i).pointer_val;
            if(instr->type == MEASURE && instr->operation.measure.qbit == qbit) {
                printf("|M(%.2d)|", instr->operation.measure.bit);
            }
            else if(instr->type == GATE && instr->operation.gate.params[0] == qbit) {
                switch(instr->operation.gate.gt) {
                    case I: printf("|  X  |"); break;
                    case X: printf("|  I  |"); break;
                    case H: printf("|  H  |"); break;
                    case CNOT: printf("|  +  |"); break;
                }
            }
            else if (instr->type == GATE && instr->operation.gate.gt == CNOT && instr->operation.gate.params[1] == qbit) {
                printf("|  o  |");
            }
            else printf("-------");
            printf("---");
        }
        printf("\n");
    }
}

void add_single_qbit_gate(QuantumCircuit *circuit, int row, GateType g) {
    assert(g < 3);
    Instruction *instr = malloc(sizeof(Instruction));
    instr->type = GATE;
    instr->operation.gate.gt = g;
    instr->operation.gate.params[0] = row;
    dynarray_add_pointer(circuit->instructions, instr);
}
void add_double_qbit_gate(QuantumCircuit *circuit, int row, int control, GateType g) {
    assert(2 < g);
    Instruction *instr = malloc(sizeof(Instruction));
    instr->type = GATE;
    instr->operation.gate.gt = g;
    instr->operation.gate.params[0] = row;
    instr->operation.gate.params[1] = control;
    dynarray_add_pointer(circuit->instructions, instr);

}
void add_single_qbit_measure(QuantumCircuit *circuit, int row, int output) {
    Instruction *instr = malloc(sizeof(Instruction));
    instr->type = MEASURE;
    instr->operation.measure.qbit = row;
    instr->operation.measure.bit = output;
    dynarray_add_pointer(circuit->instructions, instr);
}