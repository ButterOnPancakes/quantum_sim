#include "../builder/emms.h"
#include "../../utils/gnuplot.h"
#include "../../utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

int main() {
    srand(42); 
    int max_qubits = 30;
    int step = 1;
    int min_qubits = 2;
    
    int num_points = (max_qubits - min_qubits) / step + 1;
    double *x_qubits = malloc(num_points * sizeof(double));
    double *y_none = malloc(num_points * sizeof(double));
    double *y_full = malloc(num_points * sizeof(double));

    printf("EMMS Entanglement Analysis (GHZ State creation)\n");
    printf("%-8s | %-15s | %-15s | %-10s\n", "Qubits", "None (Nodes)", "Full (Nodes)", "Depth");
    printf("---------+-----------------+-----------------+-----------\n");

    int idx = 0;
    for (int n_qubits = min_qubits; n_qubits <= max_qubits; n_qubits += step) {
        // Create GHZ State Circuit: H(0), CNOT(0,1), CNOT(1,2), ...
        Node *circuit = create_circuit(n_qubits);
        
        // H(0)
        Node *h = create_h_gate();
        circuit = add_gate_to_circuit(circuit, h, 0);

        // CNOTs
        for (int i = 0; i < n_qubits - 1; i++) {
            circuit = add_cnot_to_circuit(circuit, i, i+1);
        }

        y_none[idx] = (double)count_nodes(circuit);

        // Optimize
        Node *optimized = copy_node(circuit);
        optimized = full_optimize(optimized);
        y_full[idx] = (double)count_nodes(optimized);
        int depth = tree_depth(optimized);

        x_qubits[idx] = (double)n_qubits;
        printf("%-8d | %-15.0f | %-15.0f | %-10d\n", 
               n_qubits, y_none[idx], y_full[idx], depth);

        free_node(optimized, true);
        free_node(circuit, true);
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
