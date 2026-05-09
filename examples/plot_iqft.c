#include "../builder/registers.h"
#include "../simulator/operations.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void iqft(QuantumRegister *qreg) {
    int m = qregister_get_nb_qbits(qreg);
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


/**
 * @brief Computes (base^exp) % mod classically.
 * Uses __int128 to prevent overflow during intermediate multiplication.
 */
int64 power_mod(int64 base, int64 exp, int64 mod) {
    int64 res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (int64)(((int64)res * base) % mod);
        base = (int64)(((int64)base * base) % mod);
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

void plot_qregister(QuantumRegister *qreg, char* name, char* title, char *xaxis) {
    int m = qregister_get_nb_qbits(qreg);
    FILE *graph = popen("gnuplot", "w");
    if(graph == NULL) printf("WTH\n");

    fprintf(graph, "set terminal png\n");
    fprintf(graph, "set output 'logs/%s.png'\n", name);
    fprintf(graph, "set title '%s'\n", title);

    fprintf(graph, "set xlabel '%s'\n", xaxis);
    fprintf(graph, "set xrange [0:%llu]\n", 1ULL << m);
    fprintf(graph, "set ylabel 'Probability of obtaining it'\n");
    fprintf(graph, "set yrange [0:1]\n");

    fprintf(graph, "set style data histograms\n"); fprintf(graph, "set style fill solid 0.5\n");
    
    fprintf(graph, "plot '-' with boxes\n");

    for(int64 i = 0; i < 1ULL << m; i++) {
        double proba = cabs(qregister_get_amplitude(qreg, i));
        fprintf(graph, "%ld %lf\n", i, proba);
    }

    pclose(graph);
}

void plot_spectrum(QuantumRegister *qreg, int64 a, int64 N, char* name, char* title, char *xaxis) {
    int m = qregister_get_nb_qbits(qreg);
    FILE *graph = popen("gnuplot", "w");
    if(graph == NULL) printf("WTH\n");

    fprintf(graph, "set terminal png\n");
    fprintf(graph, "set output 'logs/%s.png'\n", name);
    fprintf(graph, "set title '%s'\n", title);

    fprintf(graph, "set xlabel '%s'\n", xaxis);
    fprintf(graph, "set xrange [0:%d]\n", 16);
    fprintf(graph, "set ylabel 'Probability of obtaining it'\n");
    fprintf(graph, "set yrange [0:1]\n");

    fprintf(graph, "set style data histograms\n"); fprintf(graph, "set style fill solid 0.5\n");
    
    fprintf(graph, "plot '-' with boxes\n");

    double* amplitudes = calloc(1 << m, sizeof(double));
    for(int64 i = 0; i < 1ULL << m; i++) {
        double proba = cabs(qregister_get_amplitude(qreg, i))*cabs(qregister_get_amplitude(qreg, i));
        int64 index = find_order_cfa(i, m, a, N);
        amplitudes[index] += proba;
        //fprintf(graph, "%lu %lf\n", find_order_cfa(i, m, a, N), proba);
    }

    for(int64 i = 0; i < 1 << m; i++) {
        fprintf(graph, "%lu %lf\n", i, amplitudes[i]);
    }

    free(amplitudes);

    pclose(graph);
}


int main() {
    int N = 31, a = 5;
    int x = 1, r = 3;
    int m = 5;
    QuantumRegister *qreg = qregister_create(m);
    for(int i = 0; i < 1 << m; i++) {
        if((i - x) % r == 0) {
            printf("%d\n",i);
            qregister_set_amplitude(qreg, i, 1);
        }
        else {
            qregister_set_amplitude(qreg, i, 0);
        }
    }
    qregister_normalise(qreg);

    plot_qregister(qreg, "before_iqft", "Before IQFT (N = 31, a = 5, x_0 = 2, r = 3)", "Value obtainable (x_0 + lr)");
    iqft(qreg);
    plot_qregister(qreg, "after_iqft", "After IQFT (N = 31, a = 5, x_0 = 2, r = 3)", "Value obtainable (y = 2^m k/r)");
    plot_spectrum(qreg, a, N, "after_traitement", "After Traitement (N = 31, a = 5, x_0 = 2, r = 3)", "r obtainable");

    qregister_free(qreg);

    return EXIT_SUCCESS;
}