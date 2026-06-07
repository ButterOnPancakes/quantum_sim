#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <complex.h>

#include "../simulator/base_sim.h"
#include "../builder/circuit_old.h"

// Fonction oracle factice pour générer une charge de calcul similaire à Shor
// Fonction oracle forçant le CPU à calculer a^x mod N
bool shor_workload_oracle(int *t, int n) {
    int n_control = n / 2;
    
    // 1. Reconstruire l'entier x à partir du registre de contrôle
    // (On suppose que la première moitié des bits représente x)
    unsigned long long x = 0;
    for (int i = 0; i < n_control; i++) {
        if (t[i]) {
            x |= (1ULL << (n_control - 1 - i));
        }
    }
    
    // 2. Paramètres de Shor (exemples arbitraires pour le benchmark)
    unsigned long long a = 7;
    unsigned long long N_val = 15; 
    unsigned long long result = 1;
    
    // 3. Exponentiation modulaire réelle : result = a^x mod N
    // Cette boucle empêche le compilateur de court-circuiter le calcul
    a = a % N_val;
    while (x > 0) {
        if (x % 2 == 1) {
            result = (result * a) % N_val;
        }
        x = x >> 1;
        a = (a * a) % N_val;
    }
    
    // On retourne un booléen basé sur le résultat pour forcer 
    // le compilateur à conserver le calcul (effet de bord requis par get_oracle_matrix)
    return (result % 2 == 0); 
}

void run_shor_benchmark() {
    FILE *file = fopen("benchmark_shor.csv", "w");
    if (file == NULL) {
        perror("Erreur lors de la création du fichier CSV");
        exit(EXIT_FAILURE);
    }
    
    // En-tête du fichier CSV
    fprintf(file, "Nombre de qubits,Temps sans parallelisation,Temps 2 threads,Temps 4 threads,Temps 8 threads\n");
    printf("Debut du benchmark...\n");

    // Limite fixée à 12 qubits pour éviter une saturation de la mémoire (RAM > 16Go requise au-delà)
    for (int n_qbits = 4; n_qbits <= 13; n_qbits += 1) {
        printf("Test pour %d qubits en cours...\n", n_qbits);
        
        QuantumCircuit *circuit = circuit_create(n_qbits);
        int n_control = n_qbits / 2;

        // 1. Superposition du registre de contrôle
        for (int i = 0; i < n_control; i++) {
            add_single_qbit_gate(circuit, i, H);
        }

        // 2. Application de la matrice géante (simule l'exponentiation modulaire)
        add_multiple_qbit_gate(circuit, 0, n_qbits, ORACLE, shor_workload_oracle);

        // 3. Simulation de la structure de la QFT inverse (Hadamard + portes contrôlées)
        for (int i = 0; i < n_control; i++) {
            add_single_qbit_gate(circuit, i, H);
            for (int j = i + 1; j < n_control; j++) {
                // On utilise CNOT ici pour simuler la charge matricielle des Controlled-Phase
                add_double_qbit_gate(circuit, j, i, CNOT);
            }
        }

        // 4. Mesures
        for (int i = 0; i < n_control; i++) {
            add_single_qbit_measure(circuit, i, i);
        }

        double execution_times[4] = {0};
        int thread_configs[] = {1, 2, 4, 8};

        // Exécution pour chaque configuration de threads
        for (int t = 0; t < 4; t++) {
            omp_set_num_threads(thread_configs[t]);
            
            float complex *statevector = init_zero_state(n_qbits);
            int *bits_out = NULL;
            
            // Le timer est déjà intégré dans circuit_execute
            execution_times[t] = circuit_execute(circuit, statevector, &bits_out);
            
            free(statevector);
            if (bits_out) free(bits_out);
        }

        // Écriture dans le CSV
        fprintf(file, "%d,%.6f,%.6f,%.6f,%.6f\n", 
                n_qbits, execution_times[0], execution_times[1], execution_times[2], execution_times[3]);
        fflush(file); // Sécurise l'écriture à chaque itération

        circuit_free(circuit);
    }

    fclose(file);
    printf("Benchmark termine. Resultats exportes dans 'benchmark_shor.csv'.\n");
}

int main() {
    run_shor_benchmark();
    return 0;
}