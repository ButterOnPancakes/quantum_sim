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

    ClassicalRegister *cregister = create_cregister(3);
    QuantumRegister *qregister = create_qregister(3);
    QuantumCircuit *qc = create_circuit(qregister, cregister);

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

    circuit_execute(qc, true);

    print_qregister(stdout, qc->qregister);
    print_cregister(stdout, qc->cregister);

    double x[8];
    double y[8];
    for (int i = 0; i < 8; i++) {
        x[i] = i;
        y[i] = cabs(qregister->statevector[i]) * cabs(qregister->statevector[i]);
    }

    graph g = init_graph("Quantum Circuit Execution", "Qubit Index", "Probability Amplitude");
    histogram(g, x, y, 8, "");
    close_graph(g);

    destroy_circuit(qc);
    free_cregister(cregister);
    free_qregister(qregister);

    return EXIT_SUCCESS;
}