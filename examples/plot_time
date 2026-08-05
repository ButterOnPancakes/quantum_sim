#include "../builder/registers.h"
#include "../simulator/operations.h"
#include "../utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>

#include <omp.h>

#include <time.h>

double execute_circuit(int nb_qubits, int nb_gates) {
    QuantumRegister *qreg = qregister_create(nb_qubits);

    double t0 = now_seconds();
    for(int i = 0; i < nb_gates; i++) {
        int type = rand()%3;
        int target = rand()%nb_qubits;
        if(type == 2) { //Measure
            qregister_measure(qreg, target, NULL, 0);
        }
        else {
            int ctrl = rand()%nb_qubits;
            if(type == 1 && target != ctrl) {
                apply_cnot(qreg, ctrl, target);
            }
            else {
                apply_gate_hadamard(qreg, target);
            }
        }
    }
    double t1 = now_seconds();
    qregister_free(qreg);

    return t1 - t0;
}

double get_avg_time(int nb_qubits, int nb_gates, int N) {
    double t = 0;
    for(int i = 0; i < N; i++) {
        t += execute_circuit(nb_qubits, nb_gates);
    }
    return t / (double) N;
}

int main() {
    srand(time(NULL));
    omp_set_num_threads(omp_get_max_threads());

    FILE *graph = popen("gnuplot", "w");
    if(graph == NULL) printf("WTH\n");

    int N = 20;
    int min_qubits = 2, max_qubits = 25;
    int min_gates = 10, max_gates = 50;

    fprintf(graph, "set terminal png\n");
    fprintf(graph, "set output 'logs/time_complexity_omp.png'\n");
    fprintf(graph, "set title 'Time Complexity (multithreading)'\n");

    fprintf(graph, "set xlabel 'Number of Qubits'\n"); //fprintf(graph, "set xrange [0:%d]\n", max);
    fprintf(graph, "set ylabel 'Time (s) (log scale)'\n"); //fprintf(graph, "set yrange [0:1]\n");
    fprintf(graph, "set logscale y 10\n");
    
    fprintf(graph, "plot ");
    for(int nb_gates = min_gates; nb_gates <= max_gates; nb_gates += 10) {
        fprintf(graph, "'-' with linespoints title '%d gates'%s", 
                nb_gates, 
                (nb_gates + 10 <= max_gates) ? ", " : "\n");
    }
    printf("\n");
    fprintf(graph, "\n");

    for(int nb_gates = min_gates; nb_gates <= max_gates; nb_gates += 10) {
        for(int nb_qubits = min_qubits; nb_qubits <= max_qubits; nb_qubits++) {
            printf("Started Execution for nb_qubit = %d and nb_gates = %d\n", nb_qubits, nb_gates);
            fprintf(graph, "%u %lf\n", nb_qubits, get_avg_time(nb_qubits, nb_gates, N));
        }
        fprintf(graph, "e\n");
    }

    pclose(graph);
    return EXIT_SUCCESS;
}