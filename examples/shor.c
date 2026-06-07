#include "../builder/registers.h"
#include "../simulator/operations.h"

#include "../utils/utils.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void apply_superposition(QuantumRegister *qreg, int m) {
    for(int i = 0; i < m; i++) {
        apply_gate_hadamard(qreg, i);
    }
}

void apply_oracle(QuantumRegister *qreg, int m, int n, int64 a, int64 N) {
    for(int i = 0; i < m; i++) {
        apply_controlled_prod_exp(qreg, i, m, n, a, N, 1ULL << i);
    }
}

ClassicalRegister *measure_snd(QuantumRegister *qreg, int m, int n) {
    ClassicalRegister *creg = cregister_create(n);
    for(int i = 0; i < n; i++) {
        qregister_measure(qreg, i + m, creg, i);
    }
    return creg;
}

void apply_c_iqft(QuantumRegister *qreg, int m) {
    for(int i = 0; i < m / 2; i++) {
        apply_swap(qreg, i, m - i - 1);
    }

    for(int j = m - 1; j >= 0; j--) {
        for(int i = m - 1; i > j; i--) {
            double phase = -2 * M_PI / (1 << (i - j + 1));
            apply_controlled_rotation(qreg, i, j, phase);
        }
        apply_gate_hadamard(qreg, j);
    }
}

ClassicalRegister *measure_fst(QuantumRegister *qreg, int m) {
    ClassicalRegister *creg = cregister_create(m);
    for(int i = 0; i < m; i++) {
        qregister_measure(qreg, i, creg, i);
    }
    return creg;
}

/**
 * @brief Computes (base^exp) % mod classically.
 * Uses __int128 to prevent overflow during intermediate multiplication.
 */
int64 power_mod(int64 base, int64 exp, int64 mod) {
    int64 res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (int64)(((__int128)res * base) % mod);
        base = (int64)(((__int128)base * base) % mod);
        exp /= 2;
    }
    return res;
}

/**
 * @brief Performs Continued Fraction Analysis to find the order r.
 * Given y/2^m, finds the denominator q < N such that a^q = 1 (mod N).
 */
int64 find_order_cfa(int64 y, int64 Q, int64 a, int64 N) {
    int64 x = y;
    int64 q_val = Q;
    
    // Convergents are represented as p_n / q_n
    // We use the recurrence relation:
    // p_n = a_n * p_{n-1} + p_{n-2}
    // q_n = a_n * q_{n-1} + q_{n-2}
    int64 p_prev2 = 0, p_prev1 = 1;
    int64 q_prev2 = 1, q_prev1 = 0;

    while (x != 0) {
        int64 a_coeff = q_val / x;  // The integer part of the current fraction
        int64 remainder = q_val % x;
        
        // Calculate current convergent
        int64 p_curr = a_coeff * p_prev1 + p_prev2;
        int64 q_curr = a_coeff * q_prev1 + q_prev2;

        // Prepare for next iteration
        q_val = x;
        x = remainder;
        
        p_prev2 = p_prev1; p_prev1 = p_curr;
        q_prev2 = q_prev1; q_prev1 = q_curr;

        // The numerator p_curr of the reciprocal expansion is the denominator of the original fraction
        if (p_curr > 0 && p_curr < N) {
            // Check if this candidate (or a simple multiple) is the true period
            // power_mod is your modular exponentiation function
            if (power_mod(a, p_curr, N) == 1) {
                return p_curr;
            }
            
            // Sometimes the quantum result gives a factor of the period
            if (p_curr * 2 < N && power_mod(a, p_curr * 2, N) == 1) {
                return p_curr * 2;
            }
        }
        
        // If the candidate order exceeds N, we've gone too far
        if (p_curr >= N) break;
    }
    
    return 0; // Failure
}

int64 order_finding_c(int m, int n, int64 a, int64 N) {
    QuantumRegister *fst_reg = qregister_create(m);
    QuantumRegister *snd_reg = qregister_create(n);
    qregister_set_number(snd_reg, 1);

    QuantumRegister *qreg = qregister_fuse(fst_reg, snd_reg);
    qregister_free(fst_reg); qregister_free(snd_reg);

    apply_superposition(qreg, m);
    apply_oracle(qreg, m, n, a, N);
    ClassicalRegister *snd_creg = measure_snd(qreg, m, n);
    apply_c_iqft(qreg, m);

    ClassicalRegister *fst_creg = measure_fst(qreg, m);
    
    int64 y = cregister_calc_number(fst_creg);

    cregister_free(fst_creg);
    cregister_free(snd_creg);
    qregister_free(qreg);
    return y;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int64 N = 15;
    if(argc > 1) N = atoi(argv[1]);

    int64 a = 2 + rand()%(N-2);

    if(gcd(a, N) != 1) {
        printf("Lucky ! %ld = %ld * %ld\n", N, gcd(a, N), N/gcd(a, N));
        return EXIT_SUCCESS;
    }

    int n = (int)ceil(log2(N));
    int m = 2 * n;
    
    int64 p = 1;
    while(p == 1 || p == N) {
        int64 y = order_finding_c(m, n, a, N);
        if (y == 0) continue;

        int64 r = find_order_cfa(y, 1ULL << m, a, N);
        printf("Measured y: %ld, Candidate r: %ld\n", y, r);

        if(r == 0 || r % 2 == 1) {
            printf("r is 0 or odd, retrying...\n");
            continue;
        }

        int64 val = power_mod(a, r / 2, N);
        if (val == N - 1) {
            printf("Trivial case (a^(r/2) == -1 mod N), retrying...\n");
            continue;
        }
        p = gcd(val - 1, N);
    }
    printf("%ld = %ld * %ld\n", N, p, N/p);
    
    return EXIT_SUCCESS;
}