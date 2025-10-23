#include "circuit.h"
#include "../matrices/matrices.h"
#include "../utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>

bool f(int *t, int n) {
    return t[0] == 1 && t[1] == 1 && t[2] == 1;
}

int main() {
    int n = 3;
    int N = fast_exp_i(2, n);
    Matrix *statevector = matrix_zero(N, 1);
    matrix_set(statevector, 0, 0, 1.0);

    int l = floor(sqrt(N) * M_PI / 4.);

    QuantumCircuit *qc = create_circuit(n);

    for(int i = 0; i < n; i++) {
        add_single_qbit_gate(qc, i, H);
    }

    for(int i = 0; i < l; i++) {
        add_multiple_qbit_gate(qc, 0, n, ORACLE, &f);
        for(int k = 0; k < n; k++) {
            add_single_qbit_gate(qc, k, H);
        }
        add_multiple_qbit_gate(qc, 0, n, S0, NULL);
        for(int k = 0; k < n; k++) {
            add_single_qbit_gate(qc, k, H);
        }
    }

    for(int i = 0; i < n; i++) {
        add_single_qbit_measure(qc, i, i);
    }

    print_circuit(qc);

    int *bits = circuit_execute(qc, &statevector);
    print_list(bits, n);
    free(bits);

    matrix_print(statevector);

    printf("%lf\n", matrix_norm(statevector));

    destroy_circuit(qc);
    free(qc);

    matrix_free(statevector);
}