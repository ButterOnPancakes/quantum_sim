#include "../utils/gnuplot.h"
#include "../utils/utils.h"
#include "../builder/operations.h"
#include "../builder/circuit.h"
#include "../simulator/emms_sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <string.h>

int main() {
    srand(time(NULL));
    
    int max_qubits = 30;
    int step = 1;
    int min_qubits = 2;
    double ratio = 1 / 2.;

    int nb_gates = 20;

    printf("Ratio number of CNOT / number of gates : %lf\n", ratio);
    printf("Size of a node in memory : %lu\n", sizeof(Node));

    printf("EMMS Entanglement Analysis (GHZ State creation)\n");
    printf("%s, %s, %s, %s\n", "Qubits", "Memory Not Optimised", "Memory after optimisation", "Memory of naive");
    for (int n_qubits = min_qubits; n_qubits <= max_qubits; n_qubits += step) {
        QuantumCircuit *qc = circuit_create(n_qubits);
        

        for (int i = 0; i < nb_gates; i++) {
            if(rand() / (double) RAND_MAX < ratio) {
                circuit_add_cnot_gate(qc, 0, 1);
            }
            else {
                circuit_add_h_gate(qc, 0);
            }
        }

        int mem_no_opti = count_nodes(qc->root) * sizeof(Node);

        Node *optimized = copy_node(qc->root);
        optimized = full_optimize(optimized);
        
        int mem_opti = count_nodes(optimized) * sizeof(Node);
        
        long long unsigned mem_naif = (1 << n_qubits) * (nb_gates + 4 * rand() / (double) RAND_MAX);

        circuit_optimize(qc);

        printf("%d, %d, %d, %llu\n", 
               n_qubits, mem_no_opti, mem_opti, mem_naif);

        free_node(optimized, true);
        circuit_free(qc);
    }

    return 0;
}
