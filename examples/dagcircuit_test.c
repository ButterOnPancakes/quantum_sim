#include "../builder/circuit.h"
#include "../simulator/opti_sim.h"
#include "../utils/utils.h"
#include "../utils/gnuplot.h"
#include "../utils/logger.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <complex.h>
#include <string.h>

int main() {
    srand(time(NULL));

    ClassicalRegister *cregister = cregister_create(3);
    QuantumRegister *qregister = qregister_create(3);
    QuantumCircuit *qc = circuit_create(qregister, cregister);

    add_unitary_gate(qc, 0, GATE_H);
    add_control_gate(qc, 0, 1, GATE_X);
    add_custom_gate(qc, 2, (int[]){1,2}, (double complex[]){
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 0, 1,
        0, 0, 1, 0
    }, "SWAP");
    add_measure(qc, 0, 0);
    add_measure(qc, 1, 1);
    add_measure(qc, 2, 2);

    //circuit_execute(qc, true);

    qregister_print(stdout, qc->qregister);
    cregister_print(stdout, qc->cregister);

    double x[8];
    double y[8];
    for (int i = 0; i < 8; i++) {
        x[i] = i;
        y[i] = cabs(qregister->statevector[i]) * cabs(qregister->statevector[i]);
    }

    graph g = graph_create("Quantum Circuit Execution", "Qubit Index", "Probability Amplitude");
    graph_histogram(g, x, y, 8, "");
    graph_free(g);

    circuit_free(qc);
    cregister_free(cregister);
    qregister_free(qregister);

    return EXIT_SUCCESS;
}