#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <complex.h>
#include <stdbool.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../builder/circuit.h"
#include "../simulator/emms_sim.h"
#include "../builder/operations.h"
#include "../builder/emms.h"

// --- Fonctions Classiques Utilitaires ---

int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

long long mod_exp(long long base, long long exp, long long mod) {
    long long res = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (res * base) % mod;
        exp = exp >> 1;
        base = (base * base) % mod;
    }
    return res;
}

int get_period_from_measurement(int measurement, int n_control, int N) {
    if (measurement == 0) return 1;
    double phase = (double)measurement / (double)(1 << n_control);
    
    double val = phase;
    int max_denom = N;
    
    int h0 = 0, h1 = 1, h2;
    int k0 = 1, k1 = 0, k2 = 0;
    
    for (int i = 0; i < 30; i++) {
        int a = (int)val;
        double f = val - a;
        
        h2 = a * h1 + h0;
        k2 = a * k1 + k0;
        
        if (k2 > max_denom) {
            break;
        }
        
        h0 = h1; h1 = h2;
        k0 = k1; k1 = k2;
        
        if (f < 1e-10) break;
        val = 1.0 / f;
    }
    
    return k1;
}

// --- Memoire EMMS Utils ---

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

// --- Fonctions Quantiques ---

void build_iqft(QuantumCircuit *circuit, int n_control) {
    for (int i = n_control - 1; i >= 0; i--) {
        for (int j = n_control - 1; j > i; j--) {
            double angle = -M_PI / (1 << (j - i));
            circuit_add_cphase_gate(circuit, j, i, angle); // cible: i, controle: j
        }
        circuit_add_h_gate(circuit, i);
    }
    for (int i = 0; i < n_control / 2; i++) {
        circuit_add_swap_gate(circuit, i, n_control - 1 - i);
    }
}

int measure_statevector(float complex *statevector, int n_control, int n_qbits) {
    uint64_t dim = 1ULL << n_qbits;
    double *probs = malloc(dim * sizeof(double));
    double sum = 0.0;
    
    for (uint64_t i = 0; i < dim; i++) {
        double p = cabs(statevector[i]);
        probs[i] = p * p;
        sum += probs[i];
    }
    
    // Tirage aleatoire
    double r = ((double)rand() / RAND_MAX) * sum;
    double accum = 0.0;
    uint64_t measured_state = 0;
    
    for (uint64_t i = 0; i < dim; i++) {
        accum += probs[i];
        if (r <= accum) {
            measured_state = i;
            break;
        }
    }
    
    free(probs);
    
    // Extraction du registre de controle (bits de poids fort)
    int n_target = n_qbits - n_control;
    return (int)(measured_state >> n_target);
}

// --- Algorithme de Shor ---

void run_shor(int N) {
    printf("=== Algorithme de Shor pour N = %d (EMMS Simulator) ===\n", N);
    
    if (N % 2 == 0) {
        printf("Facteur trivial trouve: 2\n");
        return;
    }
    
    int L = (int)ceil(log2(N));
    int n_target = L;
    int n_control = 2 * L;
    int n_qbits = n_control + n_target;
    
    printf("Ressources quantiques necessaires :\n");
    printf("- Registre de controle : %d qubits\n", n_control);
    printf("- Registre cible : %d qubits\n", n_target);
    printf("- Total : %d qubits\n\n", n_qbits);

    srand(time(NULL));
    
    // Ouverture du fichier de trace (Benchmark / Opti)
    FILE *file = fopen("execution_shor_emms.csv", "a");
    if (file != NULL) {
        // Obtenir la taille pour ecrire l'en-tete si vide
        fseek(file, 0, SEEK_END);
        if (ftell(file) == 0) {
            fprintf(file, "Tentative,N,Nombre de qubits,Temps d execution,Nombre de noeuds (sans opt),Memoire sans opt (octets),Nombre de noeuds (avec opt),Memoire avec opt (octets)\n");
        }
    }

    int attempt = 1;
    bool found = false;

    while (!found && attempt <= 5) {
        printf("\n--- Tentative %d ---\n", attempt);
        
        int a = 2 + rand() % (N - 2);
        printf("1. Choix aleatoire de a = %d\n", a);
        
        int p_trivial = gcd(a, N);
        if (p_trivial > 1) {
            printf("Facteur non-trivial trouve par chance via PGCD : %d\n", p_trivial);
            if(file) fclose(file);
            return;
        }

        printf("2. Construction du circuit quantique (EMMS)...\n");
        QuantumCircuit *circuit = circuit_create(n_qbits);

        // Initialisation : H sur les controles, et X sur le dernier bit cible pour avoir |1>
        for (int i = 0; i < n_control; i++) {
            circuit_add_h_gate(circuit, i);
        }
        circuit_add_x_gate(circuit, n_qbits - 1);

        // Application de l'Oracle de Shor
        printf("   - Ajout de l'oracle U...\n");
        circuit_add_shor_oracle(circuit, n_control, n_target, a, N);

        // IQFT
        printf("   - Ajout de l'IQFT...\n");
        build_iqft(circuit, n_control);

        // Traces pre-optimisation
        int nodes_unopt = count_nodes(circuit->root);
        size_t mem_unopt = approx_memory(circuit->root);

        printf("   - Optimisation de l'arbre EMMS...\n");
        circuit_optimize(circuit);

        // Traces post-optimisation
        int nodes_opt = count_nodes(circuit->root);
        size_t mem_opt = approx_memory(circuit->root);

        printf("   - Calcul du vecteur d'etat (Simulation)...\n");
        uint64_t dim = 1ULL << n_qbits;
        float complex *statevector = calloc(dim, sizeof(float complex));
        statevector[0] = 1.0 + 0.0 * I;

        clock_t start_time = clock();
        emms_compute_statevector(circuit, statevector, dim);
        clock_t end_time = clock();
        double exec_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

        int measured_val = measure_statevector(statevector, n_control, n_qbits);

        // Export dans le CSV
        if (file) {
            fprintf(file, "%d,%d,%d,%.6f,%d,%lu,%d,%lu\n", 
                    attempt, N, n_qbits, exec_time, nodes_unopt, (unsigned long)mem_unopt, nodes_opt, (unsigned long)mem_opt);
            fflush(file);
        }

        printf("Valeur mesuree = %d (sur %d)\n", measured_val, 1 << n_control);

        int r = get_period_from_measurement(measured_val, n_control, N);
        printf("3. Periode estimee par fractions continues : r = %d\n", r);

        if (r > 0 && r % 2 == 0 && mod_exp(a, r, N) == 1) {
            long long a_r_half = mod_exp(a, r / 2, N);
            if (a_r_half != N - 1) {
                int p = gcd(a_r_half - 1, N);
                int q = gcd(a_r_half + 1, N);
                
                if (p > 1 && p < N) {
                    printf("Facteurs trouves ! %d = %d * %d\n", N, p, N / p);
                    found = true;
                } else if (q > 1 && q < N) {
                    printf("Facteurs trouves ! %d = %d * %d\n", N, q, N / q);
                    found = true;
                } else {
                    printf("Echec : les facteurs trouves sont triviaux.\n");
                }
            } else {
                printf("Echec : a^(r/2) = -1 mod N.\n");
            }
        } else {
            printf("Echec : r impair ou periode invalide.\n");
        }

        free(statevector);
        circuit_free(circuit);

        attempt++;
    }
    
    if(file) fclose(file);
}

// Fonction de benchmark originale pour conserver la possibilite de lancer l'ancien banc de test
void convert_to_binary(int n, int *t, int len) {
    for(int i = len-1; i >= 0; i--) {
        t[i] = n % 2;
        n = n / 2;
    }
}

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

Node* create_shor_oracle_leaf(int n_qbits) {
    uint64_t dim = 1ULL << n_qbits;
    float complex *mat = calloc(dim * dim, sizeof(float complex));
    int *bits = malloc(n_qbits * sizeof(int));
    
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

void run_shor_benchmark_emms() {
    FILE *file = fopen("benchmark_shor_emms.csv", "w");
    if (file == NULL) {
        perror("Erreur lors de la creation du fichier CSV");
        exit(EXIT_FAILURE);
    }
    
    fprintf(file, "Nombre de qubits,Temps d execution,Nombre de noeuds (sans opt),Memoire sans opt (octets),Nombre de noeuds (avec opt),Memoire avec opt (octets)\n");
    printf("Debut du benchmark EMMS...\n");

    for (int n_qbits = 4; n_qbits <= 14; n_qbits += 1) {
        printf("Test pour %d qubits en cours...\n", n_qbits);
        
        QuantumCircuit *circuit = circuit_create(n_qbits);
        int n_control = n_qbits / 2;

        for (int i = 0; i < n_control; i++) {
            circuit_add_h_gate(circuit, i);
        }

        Node *oracle_node = create_shor_oracle_leaf(n_qbits);
        circuit->root = create_product(oracle_node, circuit->root);

        for (int i = 0; i < n_control; i++) {
            circuit_add_h_gate(circuit, i);
            for (int j = i + 1; j < n_control; j++) {
                circuit_add_cnot_gate(circuit, i, j);
            }
        }

        int nodes_unopt = count_nodes(circuit->root);
        size_t mem_unopt = approx_memory(circuit->root);

        circuit_optimize(circuit);

        int nodes_opt = count_nodes(circuit->root);
        size_t mem_opt = approx_memory(circuit->root);

        uint64_t dim = 1ULL << n_qbits;
        float complex *statevector = calloc(dim, sizeof(float complex));
        statevector[0] = 1.0 + 0.0 * I;

        clock_t start_time = clock();
        emms_compute_statevector(circuit, statevector, dim);
        clock_t end_time = clock();
        
        double exec_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

        fprintf(file, "%d,%.6f,%d,%lu,%d,%lu\n", 
                n_qbits, exec_time, nodes_unopt, (unsigned long)mem_unopt, nodes_opt, (unsigned long)mem_opt);
        fflush(file);

        free(statevector);
        circuit_free(circuit);
    }

    fclose(file);
    printf("Benchmark termine. Resultats exportes dans 'benchmark_shor_emms.csv'.\n");
}

int main(int argc, char **argv) {
    if (argc > 1) {
        if (strcmp(argv[1], "benchmark") == 0) {
            run_shor_benchmark_emms();
            return 0;
        } else {
            int N = atoi(argv[1]);
            if (N < 2) {
                printf("N doit etre superieur ou egal a 2.\n");
                return 1;
            }
            run_shor(N);
        }
    } else {
        printf("Usage:\n");
        printf("  %s <N>         : Lance l'algorithme de Shor complet pour factoriser N\n", argv[0]);
        printf("  %s benchmark   : Lance l'ancien benchmark EMMS (creation CSV des tailles)\n", argv[0]);
        
        // Par defaut, on lance la factorisation pour 15
        run_shor(15);
    }
    return 0;
}