#include "../builder/registers.h"
#include "../simulator/operations.h"
#include "../utils/utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <complex.h>
#include <stdint.h>

// Standard Euclidean algorithm for GCD
uint64_t gcd(uint64_t a, uint64_t b) {
    while (b) {
        a %= b;
        uint64_t tmp = a; a = b; b = tmp;
    }
    return a;
}

// Modular exponentiation: (base^exp) % mod
uint64_t power(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (unsigned __int128)res * base % mod;
        base = (unsigned __int128)base * base % mod;
        exp /= 2;
    }
    return res;
}

/**
 * @brief Applies the modular exponentiation transformation.
 * |x>|y> -> |x>(y * a^x mod N)
 * This implementation is simplified for the simulator by modifying the state vector directly.
 */
void apply_mod_exp(QuantumRegister *qreg, int m, int a, int N) {
    uint64_t size = qreg->size;
    double complex *new_array = calloc(size, sizeof(double complex));
    if (!new_array) {
        fprintf(stderr, "Memory allocation failed in apply_mod_exp\n");
        return;
    }
    uint64_t m_mask = (1ULL << m) - 1;

    for (uint64_t i = 0; i < size; i++) {
        if (cabs(qreg->array[i]) < 1e-18) continue;

        uint64_t x = i & m_mask;
        uint64_t y = i >> m;

        // Shor's algorithm usually initializes the second register to |1>
        // and we compute |x> |1 * a^x mod N>.
        // Here we handle the general case for any |y> < N.
        if (y < (uint64_t)N) {
            uint64_t new_y = ((unsigned __int128)y * power(a, x, N)) % N;
            uint64_t new_idx = (new_y << m) | x;
            new_array[new_idx] += qreg->array[i];
        } else {
            // States with y >= N are left unchanged
            new_array[i] += qreg->array[i];
        }
    }

    for (uint64_t i = 0; i < size; i++) {
        qreg->array[i] = new_array[i];
    }
    free(new_array);
}

/**
 * @brief Find the period r from the measured value y and the modulus M = 2^m.
 * Uses the continued fraction expansion of y/M to find candidates for r.
 */
uint64_t find_period_from_measurement(uint64_t y, uint64_t M, int N, int a) {
    if (y == 0) return 0;
    
    uint64_t p = y, q = M;
    uint64_t a_cf[100];
    int n_cf = 0;
    
    // Compute continued fraction coefficients
    while (q != 0 && n_cf < 100) {
        a_cf[n_cf++] = p / q;
        uint64_t tmp = p % q; p = q; q = tmp;
    }

    // Check convergents h/k
    for (int i = 0; i < n_cf; i++) {
        uint64_t h_prev = 1, k_prev = 0;
        uint64_t h = a_cf[0], k = 1;
        for (int j = 1; j <= i; j++) {
            uint64_t h_next = a_cf[j] * h + h_prev;
            uint64_t k_next = a_cf[j] * k + k_prev;
            h_prev = h; k_prev = k; h = h_next; k = k_next;
        }
        
        // Candidate r is the denominator k
        if (k > 1 && k < N) {
            if (power(a, k, N) == 1) return k;
        }
    }
    
    // Also check small multiples of the denominator if N is small
    return 0;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int N = 15;
    if (argc >= 2) {
        N = atoi(argv[1]);
    }

    if (N < 2) return 0;
    if (N % 2 == 0) {
        printf("N=%d is even. A factor is 2.\n", N);
        return 0;
    }

    // Step 0: Choose a random 'a' coprime to N
    int a;
    do {
        a = 2 + rand() % (N - 2);
    } while (gcd(a, N) != 1);

    printf("--- Shor's Algorithm Implementation ---\n");
    printf("Target N = %d, chosen base a = %d\n", N, a);

    // Number of qubits for register 1 (m) and register 2 (n)
    // We need N^2 <= 2^m < 2N^2
    int n = (int)ceil(log2(N));
    int m = 2 * n; 
    int total_qbits = m + n;

    if (total_qbits > 20) {
        printf("Warning: Total qubits (%d) might exceed simulator memory limits.\n", total_qbits);
    }

    printf("Using %d qubits: Register 1 (m=%d), Register 2 (n=%d)\n", total_qbits, m, n);

    // Create Quantum Register
    QuantumRegister *qreg = qregister_create(total_qbits);

    // Initial state: |0>|1>
    // Register 1 is |0>, Register 2 is |1> (qubits m to m+n-1)
    apply_gate_x(qreg, m); 

    // Step 1: Create superposition in Register 1
    printf("Applying Hadamard gates to Register 1...\n");
    apply_n_hadamard(qreg, 0, m);

    // Step 2: Modular Exponentiation: |x>|1> -> |x>|a^x mod N>
    printf("Applying modular exponentiation (a^x mod N)...\n");
    apply_mod_exp(qreg, m, a, N);

    // Step 3: Apply Inverse QFT to Register 1
    printf("Applying Inverse QFT to Register 1...\n");
    apply_iqft(qreg, 0, m);

    // Step 4: Measure Register 1
    printf("Measuring Register 1...\n");
    ClassicalRegister *creg = cregister_create(m);
    for (int i = 0; i < m; i++) {
        qregister_measure(qreg, i, creg, i);
    }
    
    uint64_t y = cregister_calc_number(creg);
    uint64_t M = 1ULL << m;
    printf("Measured value y = %lld (M = 2^m = %lld)\n", y, M);
    printf("Phase estimate y/M = %f\n", (double)y / M);

    // Step 5: Post-processing to find factors
    uint64_t r = find_period_from_measurement(y, M, N, a);

    if (r == 0) {
        printf("Failed to find period r. The measurement did not yield enough information.\n");
    } else {
        printf("Found period r = %lld\n", r);
        if (r % 2 != 0) {
            printf("Period r is odd. Cannot proceed with this 'a'.\n");
        } else {
            uint64_t val = power(a, r / 2, N);
            if (val == N - 1) {
                printf("a^(r/2) is congruent to -1 (mod N). Trivial factors found.\n");
            } else {
                uint64_t f1 = gcd(val - 1, N);
                uint64_t f2 = gcd(val + 1, N);
                if (f1 > 1 && f1 < N) {
                    printf("SUCCESS: Found factors %lld and %lld\n", f1, N / f1);
                } else if (f2 > 1 && f2 < N) {
                    printf("SUCCESS: Found factors %lld and %lld\n", f2, N / f2);
                } else {
                    printf("Failed to find non-trivial factors.\n");
                }
            }
        }
    }

    qregister_free(qreg);
    cregister_free(creg);
    return 0;
}
