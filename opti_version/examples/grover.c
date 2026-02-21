#include "../builder/circuit.h"
#include "../simulator/opti_sim.h"
#include "../../utils/utils.h"
#include "../../utils/gnuplot.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include <omp.h>

/* Integers on 8 bits bc why not (Max 256)*/
const int n = 12;
unsigned int marked[] = {1};
const int nb_marked = 1;

int targets[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

/* Size 256 x 256 in a single line so 65536 */
double complex *ORACLE_MAT;
double complex *S0_MAT;

void init_matrices(int n) {
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

double run_grover(int n, int l) {
    srand(time(NULL));
    init_matrices(n);

    ClassicalRegister *cregister = cregister_create(n);
    QuantumRegister *qregister = qregister_create(n);
    QuantumCircuit *qc = circuit_create(n);

    for(int i = 0; i < n; i++) {
        add_unitary_gate(qc, i, GATE_H, 0.0);
    }

    for(int i = 0; i < l; i++) {
        add_custom_gate(qc, n, targets, ORACLE_MAT, " ORA "); // Oracle
        for(int k = 0; k < n; k++) {
            add_unitary_gate(qc, k, GATE_H, 0.0);
        }
        add_custom_gate(qc, n, targets, S0_MAT, "  S0 "); // Diffusion Operator
        for(int k = 0; k < n; k++) {
            add_unitary_gate(qc, k, GATE_H, 0.0);
        }
    }

    for(int i = 0; i < n; i++) {
        add_measure(qc, i, i);
    }

    double time = circuit_execute(qc, qregister, cregister, true);
    cregister_print(stdout, cregister);
    circuit_free(qc, false);
    cregister_free(cregister);
    
    char title[1024];
    sprintf(title, "Grover Statevector step %d", l);
    graph state = graph_create(title, "Basis States", "Amplitude");
    graph_statevector(state, qregister->statevector, 1 << qregister->nb_qbits);
    graph_free(state);

    qregister_free(qregister);

    free(ORACLE_MAT);
    free(S0_MAT);

    return time;
}

int main() {
    srand(time(NULL));

    int n = 4;
    int N = 1 << n;
    int l = (int) floor(M_PI / (4 * asin(sqrt((double) nb_marked / N))));
    printf("%d\n", l);
    
    run_grover(n, l);
    
    /*
    int Nmin = 2; // inclusive
    int Nmax = 12; // exclusive
    int amount = 1;

    double x[Nmax - Nmin];
    double y[Nmax - Nmin];
    for (int i = Nmin; i < Nmax; i++) {
        x[i - Nmin] = i;
        y[i - Nmin] = 0.0;
        for (int j = 0; j < amount; j++) {
            y[i - Nmin] += run_grover(i);
        }
        y[i - Nmin] /= amount;
        printf("n=%d time=%f\n", i, y[i - Nmin]);
    }

    graph g = graph_create("Grover Execution Time", "Number of Qubits", "Time (s)");
    graph_histogram(g, x, y, Nmax - Nmin, "");
    graph_free(g);
    */

    return 0;
}