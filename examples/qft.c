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

    int **targets = malloc_custom((n / 2) * sizeof(int*));
    for(int i = 0; i < n / 2; i++) {
        targets[i] = malloc_custom(2 * sizeof(int));
        targets[i][0] = i;
        targets[i][1] = n - i - 1;
    }

    for(int i = 0; i < n / 2; i++) {
        add_custom_gate(qc, 2, targets[i], SWAP, "SWAP");
    }

    return targets;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if(argc < 2) {
        printf("Usage: %s <number of qubits>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = atoi(argv[1]);

    Logger *logger = logger_create("qft.log");

    ClassicalRegister *cregister = cregister_create(n);
    QuantumRegister *qregister = qregister_create(n);
    QuantumCircuit *qc = circuit_create(n);

    qregister_print(logger->log_file, qregister);
    int **targets = add_qft_gate(qc, n);

    circuit_print(logger->log_file, qc);

    double time = circuit_execute(qc, qregister, cregister, true);
    printf("Execution time: %f seconds\n", time);

    logger_message(logger, "INFO", "Quantum Fourier Transform executed successfully.");
    logger_free(logger);

    circuit_free(qc);
    cregister_free(cregister);

    for(int i = 0; i < n / 2; i++) {
        free_custom(targets[i]);
    }
    free_custom(targets);

    graph g = graph_create("QFT Statevector", "Basis States", "Amplitude");
    graph_statevector(g, qregister_get_statevector(qregister), 1 << qregister_get_num_qubits(qregister));
    graph_free(g);
    
    qregister_free(qregister);

    return EXIT_SUCCESS;
}