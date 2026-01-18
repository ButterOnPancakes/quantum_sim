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
    QuantumCircuit *qc = circuit_create(3);

    add_unitary_gate(qc, 0, GATE_H, 0.0);
    add_control_gate(qc, 0, 1, GATE_X, 0.0);
    add_custom_gate(qc, 2, (int[]){1,2}, (double complex[]){
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 0, 1,
        0, 0, 1, 0
    }, "SWAP");
    add_measure(qc, 0, 0);
    add_measure(qc, 1, 1);
    add_measure(qc, 2, 2);

    circuit_execute(qc, qregister, cregister, true);

    qregister_print(stdout, qregister);
    cregister_print(stdout, cregister);
    
    graph g = graph_create("Quantum Circuit Execution", "Qubit Index", "Probability Amplitude");
    graph_statevector(g, qregister->statevector, 1 << qregister->nb_qbits);
    graph_free(g);

    circuit_free(qc);
    cregister_free(cregister);
    qregister_free(qregister);

    return EXIT_SUCCESS;
}