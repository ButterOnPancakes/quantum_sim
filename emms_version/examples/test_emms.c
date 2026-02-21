#include "../builder/emms.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

int main() {
    printf("Running EMMS Tests...\n");

    // Test 1: Identity Tree
    Node *id3 = create_circuit(3); // 3 qubits
    printf("Initial Identity Tree (3 qubits):\n");
    print_tree(id3, 0);
    
    // Test 2: Add Gate
    // Add a Hadamard on qubit 1 (middle).
    double complex mat[4] = {1./sqrt(2), 1./sqrt(2), 1./sqrt(2), -1./sqrt(2)};
    Node *h = create_leaf(mat, 1); // Hadamard
    Node *circuit = add_gate_to_circuit(id3, h, 1);
    printf("\nCircuit after adding Hadamard on qubit 1:\n");
    print_tree(circuit, 0);
    
    // Test 3: Rebuild Tree (Optimizations)
    printf("\nTesting rebuild_tree...\n");
    Node *optimized = flatten_tree(circuit);
    printf("After flatten_tree:\n");
    print_tree(optimized, 0);

    optimized = factorise_tensor(optimized);
    printf("\nAfter factorise_tensor:\n");
    print_tree(optimized, 0);

    optimized = gate_fusion(optimized);
    printf("\nAfter gate_fusion:\n");
    print_tree(optimized, 0);
    
    // Clean up
    free_node(optimized, true);

    printf("\nTests passed!\n");
    return 0;
}
