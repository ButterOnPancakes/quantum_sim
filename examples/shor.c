#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <complex.h>

#include "../builder/registers.h"
#include "../simulator/operations.h"
#include "../utils/utils.h"

// Variable globale pour transmettre dynamiquement la taille à l'oracle
int global_n_qbits = 0;

// Fonction oracle simulant la charge de calcul (adaptée pour l'API in-place)
bool shor_workload_oracle_inplace(int64 number) {
    int n_control = global_n_qbits / 2;
    
    // 1. Extraire x à partir de l'entier global 
    // Le registre de contrôle se trouve sur les n_control premiers qubits (poids faible)
    int64 x = number & ((1ULL << n_control) - 1);
    
    // 2. Paramètres factices du benchmark
    unsigned long long a = 7;
    unsigned long long N_val = 15; 
    unsigned long long result = 1;
    
    // 3. Exponentiation modulaire (charge CPU stricte)
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

void run_shor_inplace_benchmark() {
    FILE *file = fopen("benchmark_shor_inplace.csv", "w");
    if (file == NULL) {
        perror("Erreur lors de la création du fichier CSV");
        exit(EXIT_FAILURE);
    }
    
    // En-tête identique pour faciliter la fusion des données
    fprintf(file, "Nombre de qubits,Temps sans parallelisation,Temps 2 threads,Temps 4 threads,Temps 8 threads\n");
    printf("Debut du benchmark in-place...\n");

    for (int n_qbits = 4; n_qbits <= 26; n_qbits += 1) {
        printf("Test pour %d qubits en cours...\n", n_qbits);
        global_n_qbits = n_qbits; // Mise à jour pour l'oracle
        int n_control = n_qbits / 2;

        double execution_times[4] = {0};
        int thread_configs[] = {1, 2, 4, 8};

        for (int t = 0; t < 4; t++) {
            omp_set_num_threads(thread_configs[t]);
            
            // Mesure précise du temps réel écoulé multi-thread
            double start_time = omp_get_wtime(); 

            QuantumRegister *qreg = qregister_create(n_qbits);

            // 1. Superposition
            for (int i = 0; i < n_control; i++) {
                apply_gate_hadamard(qreg, i);
            }

            // 2. Application de l'oracle de charge
            apply_oracle_phase(qreg, 0, n_qbits, shor_workload_oracle_inplace);

            // 3. Simulation de la structure de la QFT inverse
            for (int i = 0; i < n_control; i++) {
                apply_gate_hadamard(qreg, i);
                for (int j = i + 1; j < n_control; j++) {
                    apply_cnot(qreg, j, i);
                }
            }

            // 4. Mesures
            ClassicalRegister *creg = cregister_create(n_control);
            for (int i = 0; i < n_control; i++) {
                qregister_measure(qreg, i, creg, i);
            }

            double end_time = omp_get_wtime();
            execution_times[t] = end_time - start_time;

            qregister_free(qreg);
            cregister_free(creg);
        }

        fprintf(file, "%d,%.6f,%.6f,%.6f,%.6f\n", 
                n_qbits, execution_times[0], execution_times[1], execution_times[2], execution_times[3]);
        fflush(file);
    }

    fclose(file);
    printf("Benchmark termine. Resultats exportes dans 'benchmark_shor_inplace.csv'.\n");
}

int main() {
    run_shor_inplace_benchmark();
    return 0;
}