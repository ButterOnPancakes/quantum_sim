#include "../computer/circuit.h"
#include "../matrices/matrices.h"
#include "../utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>

int marked[1][10] = {{0, 1, 0, 1, 0, 1, 0, 1, 0, 1}};
const int n = 10;
const int nb_marked = 1;

bool is_marked(int *t, int len) {
    bool res = false;
    for(int i = 0; i < nb_marked; i++) {
        res = true;
        for(int j = 0; j < n; j++) {
            if(t[j] != marked[i][j]) {
                res = false;
                break;
            }
        }
        if(res) return true;
    }
    return false;
}

int main() {
    int N = fast_exp_i(2, n);
    Matrix *statevector = matrix_zero(N, 1);
    matrix_set(statevector, 0, 0, 1.0);

    int l = (int) floor(M_PI / (4 * asin(sqrt((double) nb_marked / N))));

    QuantumCircuit *qc = create_circuit(n);

    for(int i = 0; i < n; i++) {
        add_single_qbit_gate(qc, i, H);
    }

    for(int i = 0; i < l; i++) {
        add_multiple_qbit_gate(qc, 0, n, ORACLE, &is_marked);
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

    //print_circuit(qc);

    int *bits = circuit_execute(qc, &statevector);
    print_list(bits, n);
    free(bits);

    destroy_circuit(qc);
    free(qc);

    matrix_free(statevector);
}