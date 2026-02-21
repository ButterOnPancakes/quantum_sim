#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <time.h>

// Include your provided headers
#include "../builder/circuit.h"
#include "../builder/register.h"
#include "../simulator/opti_sim.h"
#include "../../utils/utils.h"

// Classical GCD
int gcd(int a, int b) {
    while (b != 0) {
        int t = b; b = a % b; a = t;
    }
    return a;
}

// Classical Modular Exponentiation: (base^exp) % mod
int power_mod(int base, int exp, int mod) {
    long long res = 1;
    long long b = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (res * b) % mod;
        b = (b * b) % mod;
        exp /= 2;
    }
    return (int)res;
}

#include <stdio.h>
#include <stdbool.h>

/**
 * find_period_cfe
 * @param y: The measured integer from the counting register
 * @param Q: Total number of states (2^n_count)
 * @param a: The base used in a^x mod N
 * @param N: The number we are factoring
 * @return: The discovered period r, or 0 if failed.
 */
int find_period_cfe(long long y, long long Q, int a, int N) {
    long long x = y;
    long long q_val = Q;
    
    // Convergents are represented as p_n / q_n
    // We use the recurrence relation:
    // p_n = a_n * p_{n-1} + p_{n-2}
    // q_n = a_n * q_{n-1} + q_{n-2}
    long long p_prev2 = 0, p_prev1 = 1;
    long long q_prev2 = 1, q_prev1 = 0;

    printf("  [CFE] Expanding %lld/%lld...\n", y, Q);

    while (x != 0) {
        long long a_coeff = q_val / x;  // The integer part of the current fraction
        long long remainder = q_val % x;
        
        // Calculate current convergent
        long long p_curr = a_coeff * p_prev1 + p_prev2;
        long long q_curr = a_coeff * q_prev1 + q_prev2;

        // Prepare for next iteration
        q_val = x;
        x = remainder;
        
        p_prev2 = p_prev1; p_prev1 = p_curr;
        q_prev2 = q_prev1; q_prev1 = q_curr;

        // The denominator q_curr is our candidate for the period r
        if (q_curr > 0 && q_curr < N) {
            // Check if this candidate (or a simple multiple) is the true period
            // power_mod is your modular exponentiation function
            if (power_mod(a, q_curr, N) == 1) {
                return (int)q_curr;
            }
            
            // Sometimes the quantum result gives a factor of the period
            if (q_curr * 2 < N && power_mod(a, q_curr * 2, N) == 1) {
                return (int)(q_curr * 2);
            }
        }
        
        // If the denominator exceeds N, we've gone too far
        if (q_curr >= N) break;
    }
    
    return 0; // Failure
}

// --- Generalized Quantum Circuit Builder ---

void build_shor_generalized(QuantumCircuit *circuit, int N, int a, int n_counting, int n_target) {
    // 1. Initialize counting register to superposition
    for (int i = 0; i < n_counting; i++) {
        add_unitary_gate(circuit, i, GATE_H, 0);
    }

    // 2. Initialize target register to |1> 
    // Target register starts at index 'n_counting'. LSB is at (total_qubits - 1).
    add_unitary_gate(circuit, n_counting + n_target - 1, GATE_X, 0);

    // 3. Modular Exponentiation: Controlled-U^{2^j}
    for (int j = 0; j < n_counting; j++) {
        int p = pow(2, j);
        int a_p = power_mod(a, p, N);
        
        // The gate acts on 1 control bit + n_target bits
        int gate_qubits = 1 + n_target;
        int dim = 1 << gate_qubits;
        double complex *mat = calloc(dim * dim, sizeof(double complex));

        for (int i = 0; i < dim; i++) {
            int ctrl = (i >> n_target) & 1;
            int target = i & ((1 << n_target) - 1);
            
            int next_target = target;
            if (ctrl == 1 && target < N) {
                next_target = (int)(((long long)a_p * target) % N);
            }
            
            int next_index = (ctrl << n_target) | next_target;
            mat[next_index * dim + i] = 1.0 + 0.0 * I;
        }

        // Map circuit indices: control bit j, then all target bits
        int *q_indices = malloc(gate_qubits * sizeof(int));
        q_indices[0] = j;
        for(int k = 0; k < n_target; k++) q_indices[k+1] = n_counting + k;
        
        add_custom_gate(circuit, gate_qubits, q_indices, mat, "C-ModExp");
    }

    // 4. Inverse QFT on the counting register
    for (int j = n_counting - 1; j >= 0; j--) {
        for (int m = n_counting - 1; m > j; m--) {
            add_control_gate(circuit, m, j, GATE_PHASE, -M_PI / pow(2, m - j));
        }
        add_unitary_gate(circuit, j, GATE_H, 0);
    }

    // 5. Measure counting register
    for (int i = 0; i < n_counting; i++) {
        add_measure(circuit, i, i);
    }
}

int get_r(int N, int *a_out) {
    int a = rand()%N;
    while (gcd(a, N) != 1) {
        a = rand()%N;
    }
    printf("Using base a = %d\n", a);

    int n_target = (int)ceil(log2(N));
    int n_counting = 2 * n_target; // For high precision
    int total_qubits = n_counting + n_target;

    printf("Qubits required: %d (Counting: %d, Target: %d)\n", total_qubits, n_counting, n_target);

    if (total_qubits > 25) {
        printf("Warning: This may exceed your RAM limit!\n");
    }

    QuantumCircuit *qc = circuit_create(total_qubits);
    build_shor_generalized(qc, N, a, n_counting, n_target);

    QuantumRegister *qreg = qregister_create(total_qubits);
    ClassicalRegister *creg = cregister_create(n_counting);

    circuit_execute(qc, qreg, creg, false);

    // Convert result bits to fractional phase
    int y = 0;
    for (int i = 0; i < n_counting; i++) {
        if (creg->bits[i]) y |= (1 << (n_counting - 1 - i));
    }
    
    double phase = (double)y / (1 << n_counting);
    printf("Measured phase: %f\n", phase);

    // Note: To find 'r' from 'phase', one would typically use 
    // a Continued Fractions expansion here.

    int res = find_period_cfe(y, 1 << n_counting, a, N);
    printf("%d\n", res);

    circuit_free(qc, true);
    qregister_free(qreg);
    cregister_free(creg);

    *a_out = a;
    return res;
}

void extract_factors(int N, int a, int r) {
    printf("  [Post] Validating period r = %d...\n", r);

    // Condition 1: Period must be even
    if (r % 2 != 0) {
        printf("  [Post] Failure: Period r is odd. Cannot factorize. Re-run quantum circuit.\n");
        return;
    }

    // Condition 2: a^(r/2) mod N != N-1
    long long half_pow = power_mod(a, r / 2, N);
    if (half_pow == N - 1) {
        printf("  [Post] Failure: a^(r/2) is -1 (mod N). Trivial result. Re-run quantum circuit.\n");
        return;
    }

    // --- SUCCESS: Calculate Factors ---
    // If we passed the checks, then:
    // a^r - 1 = (a^(r/2) - 1)(a^(r/2) + 1) = 0 (mod N)
    // The factors of N are found by taking GCDs of these parts with N.

    long long factor1 = gcd(half_pow - 1, N);
    long long factor2 = gcd(half_pow + 1, N);

    printf("\n--- SUCCESS: Factors found ---\n");
    printf("Factors of %d are: %lld and %lld\n", N, factor1, factor2);
    printf("Verification: %lld * %lld = %lld\n", factor1, factor2, factor1 * factor2);
}

int main() {
    int N;
    srand(time(NULL));
    printf("Enter number to factor (N): ");
    scanf("%d", &N);

    // Basic classical checks
    if (N % 2 == 0) { printf("Factors are 2 and %d\n", N/2); return 0; }

    int a = 2;
    int r = get_r(N, &a);
    long long half_pow = power_mod(a, r / 2, N);
    while (r == 0 || half_pow == N - 1 || r % 2 != 0) {
        r = get_r(N, &a);
        half_pow = power_mod(a, r / 2, N);
    }

    extract_factors(N, a, r);

    return 0;
}