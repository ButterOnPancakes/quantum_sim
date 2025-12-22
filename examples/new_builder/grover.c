#include "../../builder/new/circuit.h"
#include "../../simulator/opti/opti_sim.h"
#include "../../utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>

/* Integers on 8 bits bc why not (Max 256)*/
const int n = 8;
unsigned int marked[] = {23, 48, 204};
const int nb_marked = 3;

int targets[8] = {0, 1, 2, 3, 4, 5, 6, 7};

/* Size 256 x 256 in a single line so 65536 */
double complex *ORACLE_MAT;
double complex *S0_MAT;

void init_matrices() {
    unsigned long size = 1 << n;

    /* Init S0 = 2|0><0| - In */
    S0_MAT = calloc(size * size, sizeof(double complex));
    for(int i = 0; i < size; i++) {
        S0_MAT[i * size + i] = -1.0;
    }
    S0_MAT[0] = 1.0;

    /* Init Oracle */
    ORACLE_MAT = calloc(size * size, sizeof(double complex));
    for(int i = 0; i < size; i++) {
        ORACLE_MAT[i * size + i] = 1.0;
    }
    for(int i = 0; i < nb_marked; i++) {
        int idx = marked[i];
        ORACLE_MAT[idx * size + idx] = -1.0;
    }
}

int main() {
    srand(time(NULL));
    init_matrices();
    int N = 1 << n;
    int l = (int) floor(M_PI / (4 * asin(sqrt((double) nb_marked / N))));

    ClassicalRegister *cregister = create_cregister(n);
    QuantumRegister *qregister = create_qregister(n);
    QuantumCircuit *qc = create_circuit(qregister, cregister);

    for(int i = 0; i < n; i++) {
        add_unitary_gate(qc, i, GATE_H);
    }

    for(int i = 0; i < l; i++) {
        add_custom_gate(qc, n, targets, ORACLE_MAT, " ORA "); // Oracle
        for(int k = 0; k < n; k++) {
            add_unitary_gate(qc, k, GATE_H);
        }
        add_custom_gate(qc, n, targets, S0_MAT, "  S0 "); // Diffusion Operator
        for(int k = 0; k < n; k++) {
            add_unitary_gate(qc, k, GATE_H);
        }
    }

    for(int i = 0; i < n; i++) {
        add_measure(qc, i, i);
    }

    circuit_execute(qc);

    print_cregister(qc->cregister);

    destroy_circuit(qc);
    free_cregister(cregister);
    free_qregister(qregister);
}