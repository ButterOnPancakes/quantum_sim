#include "../builder/circuit.h"
#include "../simulator/opti_sim.h"
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
    QuantumCircuit *qc = circuit_create(nb_qubits);

    for(int i = 0; i < nb_gates; i++) {
        int type = rand()%13;
        int target = rand()%nb_qubits;
        if(type == 12) { //Measure
            add_measure(qc, target, 0);
        }
        else {
            SingleBitGate gt = type % 6;
            bool controlled = type / 6;
            int ctrl = rand()%nb_qubits;
            if(controlled && target != ctrl) {
                add_control_gate(qc, ctrl, target, gt, 0);
            }
            else {
                add_unitary_gate(qc, target, gt, 0);
            }
        }
    }
    
    double time = circuit_execute(qc, qreg, NULL, false);

    qregister_free(qreg);
    circuit_free(qc);

    return time;
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
    int min_qubits = 2, max_qubits = 12;
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