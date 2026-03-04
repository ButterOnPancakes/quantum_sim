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
    int max_qubits = 30;
    int step = 1;
    int min_qubits = 2;
    
    int num_points = (max_qubits - min_qubits) / step + 1;
    double *x_qubits = malloc_custom(num_points * sizeof(double));
    double *y_none = malloc_custom(num_points * sizeof(double));
    double *y_full = malloc_custom(num_points * sizeof(double));
    assert(x_qubits != NULL && y_none != NULL && y_full != NULL);

    printf("EMMS Entanglement Analysis (GHZ State creation)\n");
    printf("%-8s | %-15s | %-15s | %-10s | %-8s\n", "Qubits", "None (Nodes)", "Full (Nodes)", "Depth", "Time");
    printf("---------+-----------------+-----------------+------------+--------\n");

    int idx = 0;
    for (int n_qubits = min_qubits; n_qubits <= max_qubits; n_qubits += step) {
        uint64_t dim = 1 << n_qubits;
        QuantumCircuit *qc = circuit_create(n_qubits);
        
        circuit_add_h_gate(qc, 0);

        for (int i = 0; i < n_qubits - 1; i++) {
            circuit_add_cnot_gate(qc, i, i+1);
        }

        y_none[idx] = (double)count_nodes(qc->root);

        Node *optimized = copy_node(qc->root);
        optimized = full_optimize(optimized);
        y_full[idx] = (double)count_nodes(optimized);
        int depth = tree_depth(optimized);

        circuit_optimize(qc);
        double complex *sv = calloc_custom(dim, sizeof(double complex));
        assert(sv != NULL);
        sv[0] = 1;

        double start = now_seconds();
        emms_compute_statevector(qc, sv, dim);
        double end = now_seconds();

        free(sv);

        x_qubits[idx] = (double)n_qubits;
        printf("%-8d | %-15.0f | %-15.0f | %-10d | %-8f\n", 
               n_qubits, y_none[idx], y_full[idx], depth, end - start);

        free_node(optimized, true);
        circuit_free(qc);
        idx++;
    }
    
    graph g = graph_create("EMMS Entanglement Benchmark", "Number of Qubits", "Number of Tree Nodes");
    if (g) {
        graph_plot_comparison(g, x_qubits, y_none, y_full, idx, "Unoptimized", "Optimized");
        graph_free(g);
        printf("\nEntanglement plot saved to logs/EMMS_Entanglement_Benchmark.png\n");
    }

    free(x_qubits);
    free(y_none);
    free(y_full);

    return 0;
}
