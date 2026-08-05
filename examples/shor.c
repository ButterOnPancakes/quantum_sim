#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <complex.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../simulator/base_sim.h"
#include "../builder/circuit_old.h"

// --- Fonctions Classiques Utilitaires ---

// Calcul du PGCD
int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Exponentiation modulaire : (base^exp) % mod
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

// Développement en fraction continue pour retrouver la période r
// à partir de la phase mesurée m / 2^n_control
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

// --- Fonctions Quantiques ---

// Ajout de l'IQFT (Inverse Quantum Fourier Transform)
void build_iqft(QuantumCircuit *circuit, int n_control) {
    // L'IQFT applique les portes dans l'ordre inverse de la QFT
    for (int i = n_control - 1; i >= 0; i--) {
        for (int j = n_control - 1; j > i; j--) {
            double angle = -M_PI / (1 << (j - i));
            add_cphase_gate(circuit, i, j, angle); // cible: i, contrôle: j
        }
        add_single_qbit_gate(circuit, i, H);
    }
    // Echange final pour remettre les qubits dans le bon ordre
    for (int i = 0; i < n_control / 2; i++) {
        add_swap_gate(circuit, i, n_control - 1 - i);
    }
}

// --- Algorithme de Shor ---

void run_shor(int N) {
    printf("=== Algorithme de Shor pour N = %d ===\n", N);
    
    if (N % 2 == 0) {
        printf("Facteur trivial trouve: 2\n");
        return;
    }
    
    // Calcul de L = log2(N)
    int L = (int)ceil(log2(N));
    int n_target = L;
    int n_control = 2 * L;
    int n_qbits = n_control + n_target;
    
    printf("Ressources quantiques necessaires :\n");
    printf("- Registre de controle : %d qubits\n", n_control);
    printf("- Registre cible : %d qubits\n", n_target);
    printf("- Total : %d qubits\n\n", n_qbits);

    srand(time(NULL));

    int attempt = 1;
    bool found = false;

    while (!found && attempt <= 5) {
        printf("\n--- Tentative %d ---\n", attempt);
        
        // 1. Choisir a au hasard tel que 1 < a < N
        int a = 2 + rand() % (N - 2);
        printf("1. Choix aleatoire de a = %d\n", a);
        
        // Vérifier si a et N sont premiers entre eux
        int p_trivial = gcd(a, N);
        if (p_trivial > 1) {
            printf("Facteur non-trivial trouve par chance via PGCD : %d\n", p_trivial);
            return;
        }

        // 2. Construction du circuit quantique
        printf("2. Construction et execution du circuit quantique...\n");
        QuantumCircuit *circuit = circuit_create(n_qbits);

        // Initialisation : H sur les controles, et X sur le dernier bit cible pour avoir |1>
        for (int i = 0; i < n_control; i++) {
            add_single_qbit_gate(circuit, i, H);
        }
        add_single_qbit_gate(circuit, n_qbits - 1, X);

        // Application de l'Oracle de Shor U|x>|y> = |x>|a^x y mod N>
        add_shor_oracle(circuit, n_control, n_target, a, N);

        // IQFT sur le registre de controle
        build_iqft(circuit, n_control);

        // Mesure
        for (int i = 0; i < n_control; i++) {
            add_single_qbit_measure(circuit, i, i);
        }

        float complex *statevector = init_zero_state(n_qbits);
        int *bits_out = NULL;
        
        circuit_execute(circuit, statevector, &bits_out);

        // 3. Post-traitement classique
        int measured_val = 0;
        for (int i = 0; i < n_control; i++) {
            if (bits_out[i]) measured_val |= (1 << (n_control - 1 - i));
        }

        printf("Valeur mesuree = %d (sur %d)\n", measured_val, 1 << n_control);

        int r = get_period_from_measurement(measured_val, n_control, N);
        printf("3. Periode estimee par fractions continues : r = %d\n", r);

        // Vérification de la période
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
        if (bits_out) free(bits_out);
        circuit_free(circuit);

        attempt++;
    }
}

int main(int argc, char **argv) {
    int N = 15;
    if (argc > 1) {
        N = atoi(argv[1]);
    }
    
    if (N < 2) {
        printf("N doit etre superieur ou egal a 2.\n");
        return 1;
    }
    
    run_shor(N);
    return 0;
}