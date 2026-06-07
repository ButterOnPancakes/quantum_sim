#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <complex.h>

#include "../builder/circuit.h"
#include "../simulator/emms_sim.h"
#include "../builder/operations.h"
#include "../builder/emms.h"

// --- Fonctions Utilitaires ---

size_t approx_memory(Node *node) {
    if (node == NULL) return 0;
    
    size_t mem = sizeof(Node);
    if (node->gt == LEAF) {
        if (node->data.leaf.mat != NULL) {
            mem += (node->dim * node->dim) * sizeof(float complex);
        }
    } else {
        if (!node->is_zero && !node->is_identity) {
            mem += approx_memory(node->data.operation.left_child);
            mem += approx_memory(node->data.operation.right_child);
        }
    }
    return mem;
}

void convert_to_binary(int n, int *t, int len) {
    for(int i = len-1; i >= 0; i--) {
        t[i] = n % 2;
        n = n / 2;
    }
}

// --- Oracle de Charge (Identique à l'ancien benchmark) ---

bool shor_workload_oracle(int *t, int n) {
    int n_control = n / 2;
    unsigned long long x = 0;
    for (int i = 0; i < n_control; i++) {
        if (t[i]) {
            x |= (1ULL << (n_control - 1 - i));
        }
    }
    unsigned long long a = 7;
    unsigned long long N_val = 15; 
    unsigned long long result = 1;
    
    a = a % N_val;
    while (x > 0) {
        if (x % 2 == 1) {
            result = (result * a) % N_val;
        }
        x = x >> 1;
        a = (a * a) % N_val;
    }
    return (result % 2 == 0); 
}

// Transforme l'oracle en un noeud LEAF géant pour l'arbre EMMS
Node* create_shor_oracle_leaf(int n_qbits) {
    uint64_t dim = 1ULL << n_qbits;
    float complex *mat = calloc(dim * dim, sizeof(float complex));
    int *bits = malloc(n_qbits * sizeof(int));
    
    // Initialisation comme une matrice identité, mais on met -1 sur la diagonale
    // si l'oracle renvoie true.
    for (uint64_t i = 0; i < dim; i++) {
        convert_to_binary(i, bits, n_qbits);
        if (shor_workload_oracle(bits, n_qbits)) {
            mat[i * dim + i] = -1.0 + 0.0 * I;
        } else {
            mat[i * dim + i] = 1.0 + 0.0 * I;
        }
    }
    
    Node *oracle = create_leaf(mat, n_qbits);
    free(mat);
    free(bits);
    return oracle;
}

// --- Fonction de Benchmark Principale ---

void run_shor_benchmark_emms() {
    FILE *file = fopen("benchmark_shor_emms.csv", "w");
    if (file == NULL) {
        perror("Erreur lors de la création du fichier CSV");
        exit(EXIT_FAILURE);
    }
    
    fprintf(file, "Nombre de qubits,Temps d execution,Nombre de noeuds (sans opt),Memoire sans opt (octets),Nombre de noeuds (avec opt),Memoire avec opt (octets)\n");
    printf("Debut du benchmark EMMS...\n");

    // On s'arrête à 13 pour les mêmes raisons de RAM que le benchmark original
    // (Une LEAF de 13 qubits demande ~536 Mo d'allocation contiguë)
    for (int n_qbits = 4; n_qbits <= 14; n_qbits += 1) {
        printf("Test pour %d qubits en cours...\n", n_qbits);
        
        QuantumCircuit *circuit = circuit_create(n_qbits);
        int n_control = n_qbits / 2;

        // 1. Superposition du registre de contrôle
        for (int i = 0; i < n_control; i++) {
            circuit_add_h_gate(circuit, i);
        }

        // 2. Application de la matrice géante (simule l'exponentiation modulaire)
        Node *oracle_node = create_shor_oracle_leaf(n_qbits);
        circuit->root = create_product(oracle_node, circuit->root);

        // 3. Simulation de la structure de la QFT inverse
        for (int i = 0; i < n_control; i++) {
            circuit_add_h_gate(circuit, i);
            for (int j = i + 1; j < n_control; j++) {
                circuit_add_cnot_gate(circuit, i, j); // controle: i, cible: j
            }
        }

        // Relevés AVANT optimisation
        int nodes_unopt = count_nodes(circuit->root);
        size_t mem_unopt = approx_memory(circuit->root);

        // Optimisation de l'arbre
        circuit_optimize(circuit);

        // Relevés APRES optimisation
        int nodes_opt = count_nodes(circuit->root);
        size_t mem_opt = approx_memory(circuit->root);

        // Initialisation de l'état |0...0>
        uint64_t dim = 1ULL << n_qbits;
        float complex *statevector = calloc(dim, sizeof(float complex));
        statevector[0] = 1.0 + 0.0 * I;

        // Mesure du temps d'exécution (application de l'arbre au vecteur)
        clock_t start_time = clock();
        emms_compute_statevector(circuit, statevector, dim);
        clock_t end_time = clock();
        
        double exec_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

        // Export dans le CSV
        fprintf(file, "%d,%.6f,%d,%lu,%d,%lu\n", 
                n_qbits, exec_time, nodes_unopt, (unsigned long)mem_unopt, nodes_opt, (unsigned long)mem_opt);
        fflush(file);

        free(statevector);
        circuit_free(circuit);
    }

    fclose(file);
    printf("Benchmark termine. Resultats exportes dans 'benchmark_shor_emms.csv'.\n");
}

int main() {
    run_shor_benchmark_emms();
    return 0;
}