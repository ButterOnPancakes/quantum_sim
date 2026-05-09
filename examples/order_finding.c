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
int64 find_order_cfa(int64 y, int m, int64 a, int64 N) {
    if (y == 0) return 0;

    double target = (double)y / (double)(1ULL << m);
    int64 p_prev2 = 0, p_prev1 = 1;
    int64 q_prev2 = 1, q_prev1 = 0;
    double val = target;

    // Expansion limit is typically proportional to the number of bits
    for (int i = 0; i < 2 * m; i++) {
        int64 a_i = (int64)floor(val + 1e-15);
        int64 p_curr = a_i * p_prev1 + p_prev2;
        int64 q_curr = a_i * q_prev1 + q_prev2;

        // The order r must be less than N
        if (q_curr >= N) break;

        // Check if this denominator is the order
        if (q_curr > 0 && power_mod(a, q_curr, N) == 1) {
            return q_curr;
        }

        // Update convergents
        p_prev2 = p_prev1; p_prev1 = p_curr;
        q_prev2 = q_prev1; q_prev1 = q_curr;

        double remainder = val - a_i;
        if (fabs(remainder) < 1e-15) break;
        val = 1.0 / remainder;
    }
    return 0;
}

void plot_qregister(QuantumRegister *qreg, int m, int n, char* name) {
    FILE *graph = popen("gnuplot", "w");
    if(graph == NULL) printf("WTH\n");

    fprintf(graph, "set terminal png\n");
    fprintf(graph, "set output 'logs/%s.png'\n", name);
    fprintf(graph, "set title 'Number of precision qubit impact'\n");

    fprintf(graph, "set xlabel 'Decimal Number obtainable'\n");
    fprintf(graph, "set xrange [0:%llu]\n", 1ULL << (m+n));
    fprintf(graph, "set ylabel 'Probability of obtaining it'\n");
    fprintf(graph, "set yrange [0:1]\n");

    fprintf(graph, "set style data histograms\n"); fprintf(graph, "set style fill solid 0.5\n");
    
    fprintf(graph, "plot '-' with boxes\n");

    for(int64 i = 0; i < 1ULL << (m + n); i++) {
        double proba = qregister_calc_proba_multiple(qreg, i);
        fprintf(graph, "%ld %lf\n", i, proba);
    }

    pclose(graph);
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
    
    plot_qregister(qreg, m, n, "before_QFT");
    
    apply_c_iqft(qreg, m);

    plot_qregister(qreg, m, n, "after_QFT");
    ClassicalRegister *fst_creg = measure_fst(qreg, m);
    
    int64 y = cregister_calc_number(fst_creg);

    cregister_free(fst_creg);
    cregister_free(snd_creg);
    qregister_free(qreg);
    return y;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    if(argc < 2) return EXIT_FAILURE;
    int64 N = (int64)atoll(argv[1]);
    
    int n = (int)ceil(log2(N));
    int m = n;

    int amount = 1000;
    int count = 0;
    for(int x = 0; x < amount; x++) {
        int64 a = N;
        while(gcd(a, N) != 1) {
            a = rand()%(N-1)+1;
        }

        int64 y = order_finding_c(m, n, a, N);
        int64 r = find_order_cfa(y, m, a, N);

        //printf("a = %ld ; N = %ld ; y = %ld ; y/(2^m) = %lf\n", a, N, y, (double)y / (1ULL << m));
        if (r > 0) {
            count++;
            printf("Successfully found order a = %ld, r = %ld\n", a, r);
            return EXIT_SUCCESS;
        }
        //else printf("Continued fraction failed to find order from measurement %ld\n", y);
    }

    printf("Proba : %lf\n", count / (double) amount);
    return EXIT_SUCCESS;
}