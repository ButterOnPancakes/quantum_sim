#include "../builder/circuit.h"
#include "../simulator/opti_sim.h"
#include "../../utils/utils.h"
#include "../../utils/gnuplot.h"
#include "../../utils/logger.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <complex.h>
#include <string.h>
#include <omp.h>

double complex SWAP[16] = {
    1, 0, 0, 0,
    0, 0, 1, 0,
    0, 1, 0, 0,
    0, 0, 0, 1
};

int **add_qft_gate(QuantumCircuit* qc, int n) {
    for(int i = 0; i < n; i++) {
        add_unitary_gate(qc, i, GATE_H, 0.0);
        for(int j = 2; j < n + 1 - i; j++) {
            add_control_gate(qc, i + j - 1, i, GATE_PHASE, M_PI / (1 << (j - 1)));
        }
    }

    int **targets = malloc((n / 2) * sizeof(int*));
    for(int i = 0; i < n / 2; i++) {
        targets[i] = malloc(2 * sizeof(int));
        targets[i][0] = i;
        targets[i][1] = n - i - 1;
    }

    for(int i = 0; i < n / 2; i++) {
        add_custom_gate(qc, 2, targets[i], SWAP, "SWAP");
    }

    return targets;
}

int main() {
    srand(time(NULL));

    int n = 24;

    Logger *logger = logger_create("qft.log");

    ClassicalRegister *cregister = cregister_create(n);
    QuantumRegister *qregister = qregister_create(n);
    QuantumCircuit *qc = circuit_create(n);

    qregister_print(logger->log_file, qregister);
    int **targets = add_qft_gate(qc, n);

    circuit_print(logger->log_file, qc);

    double time = circuit_execute(qc, qregister, cregister, true);
    printf("%f\n", time);

    logger_message(logger, "INFO", "Quantum Fourier Transform executed successfully.");
    logger_free(logger);

    circuit_free(qc, false);
    cregister_free(cregister);

    for(int i = 0; i < n / 2; i++) {
        free(targets[i]);
    }
    free(targets);

    graph g = graph_create("QFT Statevector", "Basis States", "Amplitude");
    graph_statevector(g, qregister->statevector, 1 << n);
    graph_free(g);
    
    qregister_free(qregister);

    return EXIT_SUCCESS;
}