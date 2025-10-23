#include "circuit.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include "../matrices/matrices.h"
#include "../utils/lists/list.h"
#include "../utils/utils.h"

bool collapse = false;

typedef struct {
    double complex matrix[2][2];
} GateMatrix;

typedef struct qgate {
    GateType type;
    bool (*f)(int *t, int n);
    int params[3]; // 0 : target qbit, 1 : control qbit or bit measured
} Gate;

typedef struct qcircuit {
    int nb_qbits;
    List* gates;
} QuantumCircuit;

QuantumCircuit *create_circuit(int n_qbits) {
    QuantumCircuit *circuit = malloc(sizeof(QuantumCircuit));
    circuit->nb_qbits = n_qbits;
    circuit->gates = list_create();
    return circuit;
}
void destroy_circuit(QuantumCircuit *circuit) {
    ListIterator iter = list_iterator_begin(circuit->gates);
    while (list_iterator_has_next(&iter)) {
        Gate *gate = list_iterator_next(&iter);
        free(gate);
    }
    list_destroy(circuit->gates);
}

void print_circuit(QuantumCircuit *circuit) {
    for(int i = 0; i < circuit->nb_qbits; i++) {
        printf("q%.2d: ", i);

        bool has_gate = false;
        ListIterator iter = list_iterator_begin(circuit->gates);
        while (list_iterator_has_next(&iter)) {
            Gate *gate = list_iterator_next(&iter);
            if(gate->type == CNOT && gate->params[1] == i) {
                printf("-|  o  |-");
                continue;
            }
            if(gate->params[0] <= i && i < gate->params[0] + gate->params[1]) {
                if(gate->type == ORACLE) {
                    printf("-| ORA |-");
                    continue;
                }
                if(gate->type == S0) {
                    printf("-|  S  |-");
                    continue;
                }
            }

            if(i != gate->params[0]) {
                printf("---------");
                continue;
            }

            switch(gate->type) {
                case ID: printf("-|  I  |-"); break;
                case X: printf("-|  X  |-"); break;
                case Z: printf("-|  Z  |-"); break;
                case H: printf("-|  H  |-"); break;
                case MEAS: printf("-|M(%.2d)|-", gate->params[1]); break;
                case CNOT: printf("-|  +  |-"); break;
                case ORACLE: printf("-| ORA |-"); break;
                case S0: printf("-|  S  |-"); break;
                default: printf("-|?????|-"); break;
            }
        }
        printf("\n");
    }
}

void add_single_qbit_gate(QuantumCircuit *circuit, int row, GateType g) {
    assert(g != MEAS);
    Gate *gate = malloc(sizeof(Gate));
    gate->type = g;
    gate->params[0] = row; gate->params[1] = -1;
    list_append(circuit->gates, gate);
}
void add_multiple_qbit_gate(QuantumCircuit *circuit, int row, int nb_qbits, GateType g, bool (*f)(int *t, int n)) {
    Gate *gate = malloc(sizeof(Gate));
    gate->type = g;
    gate->params[0] = row; gate->params[1] = nb_qbits;
    gate->f = f;
    list_append(circuit->gates, gate);
}
void add_double_qbit_gate(QuantumCircuit *circuit, int row, int control, GateType g) {
    assert(g == CNOT);
    Gate *gate = malloc(sizeof(Gate));
    gate->type = g;
    gate->params[0] = row; gate->params[1] = control;
    list_append(circuit->gates, gate);
}
void add_single_qbit_measure(QuantumCircuit *circuit, int row, int output) {
    Gate *gate = malloc(sizeof(Gate));
    gate->type = MEAS;
    gate->params[0] = row; gate->params[1] = output;
    list_append(circuit->gates, gate);
}

Operator get_corresponding_operator(GateType gate, int value) {
    switch(gate) {
        case ID : return IDENTITY;
        case X: return XGATE;
        case Z: return ZGATE;
        case H: return HADAMARD;
        case MEAS: return (value == 0) ? PROJ0 : PROJ1;
        default:
            fprintf(stderr, "This gate cant be converted to a single qbit operator\n");
            exit(EXIT_FAILURE);
            break;
    }
}
GateMatrix get_gate_matrix(Operator gt) {
    GateMatrix gm;
    switch (gt) {
        case XGATE: 
            gm.matrix[0][0] = 0; gm.matrix[0][1] = 1;
            gm.matrix[1][0] = 1; gm.matrix[1][1] = 0;
            break;
        case ZGATE:
            gm.matrix[0][0] = 1; gm.matrix[0][1] = 0;
            gm.matrix[1][0] = 0; gm.matrix[1][1] = -1;
            break;
        case HADAMARD: 
            double coef = 1/sqrt(2);
            gm.matrix[0][0] = coef; gm.matrix[0][1] = coef;
            gm.matrix[1][0] = coef; gm.matrix[1][1] = -coef; 
            break;
        case PROJ0:
            gm.matrix[0][0] = 1; 
            gm.matrix[0][1] = 0;
            gm.matrix[1][0] = 0;
            gm.matrix[1][1] = 0;
            break; 
        case PROJ1:
            gm.matrix[0][0] = 0; 
            gm.matrix[0][1] = 0;
            gm.matrix[1][0] = 0;
            gm.matrix[1][1] = 1;
            break; 
        default: 
            gm.matrix[0][0] = 1; gm.matrix[0][1] = 0;
            gm.matrix[1][0] = 0; gm.matrix[1][1] = 1;
            break;
    }

    return gm;
}

Matrix *get_oracle_matrix(int n, bool (*f)(int *t, int k)) {
    int matrix_size = fast_exp_i(2, n);
    Matrix *oracle = matrix_identity(matrix_size);
    
    int *input_bits = malloc(n * sizeof(int));
    // Only modify the -1 entries (much fewer operations)
    for (int state = 0; state < matrix_size; state++) {
        for (int i = 0; i < n; i++) {
            input_bits[i] = (state >> i) & 1;
        }
        
        if (f(input_bits, n)) {
            matrix_set(oracle, state, state, -1.0 + 0.0*I);
        }
    }
    
    free(input_bits);
    return oracle;
}
Matrix *get_S0_matrix(int n) {
    int size = fast_exp_i(2, n);
    Matrix *S0 = matrix_zero(size, size);
    
    // Start with -I (negative identity)
    for (int i = 0; i < size; i++) {
        matrix_set(S0, i, i, -1.0 + 0.0*I);
    }
    
    // Add 2|0><0|: set top-left corner to +1 instead of -1
    // This changes the (0,0) element from -1 to +1, net effect: +2 at (0,0)
    matrix_set(S0, 0, 0, 1.0);
    
    return S0;
}

Matrix *get_tensored_gate_matrix(Matrix *mat, int i, int n) {
    Matrix *gateMat;
    int k = reverse_power(matrix_nb_column(mat));
    int end = i + k;

    if(i > 0) {
        gateMat = matrix_identity(fast_exp_i(2, i));
        matrix_tensor_product(gateMat, mat, &gateMat);
    }
    else gateMat = matrix_duplicate(mat);


    if(end < n) {
        Matrix *complete_identity = matrix_identity(fast_exp_i(2, n - end));

        matrix_tensor_product(gateMat, complete_identity, &gateMat);
        matrix_free(complete_identity);
    }

    return gateMat;
}
Matrix *get_tensored_gate_array(Operator *gates, int n) {
    assert(n > 0);
    Matrix *gateMat = matrix_of_array(2, 2, get_gate_matrix(gates[0]).matrix);
    for(int i = 1; i < n; i++) {
        matrix_tensor_product_stack(gateMat, 2, 2, get_gate_matrix(gates[i]).matrix, &gateMat);
    }
    return gateMat;
}

void reset_gate_array(Operator *gates, int n) {
    for(int i = 0; i < n; i++) {
        gates[i] = IDENTITY;
    }
}

int *circuit_execute(QuantumCircuit *circuit, Matrix **statevector) {
    assert(circuit != NULL);
    srand(time(NULL));
    int n = circuit->nb_qbits;

    int *bits = calloc(n, sizeof(int));

    //Init temp gate for CNOT gates
    Operator *gates_tensored = malloc(n * sizeof(Operator));
    reset_gate_array(gates_tensored, n);

    ListIterator iter = list_iterator_begin(circuit->gates);
    while (list_iterator_has_next(&iter)) {
        Gate *gate = list_iterator_next(&iter);
        int target = gate->params[0];
        int value = 0; // Modified by measure

        if(gate->type == MEAS) {
            Matrix *tempProj0 = matrix_of_array(2, 2, get_gate_matrix(PROJ0).matrix);
            Matrix *Proj0 = get_tensored_gate_matrix(tempProj0, target, n);
            matrix_free(tempProj0);

            matrix_mult(Proj0, *statevector, &Proj0);
            matrix_print(Proj0);
            double proba0 = matrix_norm(Proj0);
            matrix_free(Proj0);

            if(rand() / (double) RAND_MAX < proba0) value = 0;
            else value = 1;

            printf("Measured %d on qbit %d with probability %.2f\n", value, target, fabs((double)value - proba0));

            bits[gate->params[1]] = value;
        }

        Matrix *gateMat;
        
        if(gate->type == CNOT || gate->type == CZ) {
            int control = gate->params[1];
            assert(control >= 0 && control < n && control != target);

            //Measured 0
            gates_tensored[control] = PROJ0;
            gates_tensored[target] = IDENTITY;
            gateMat = get_tensored_gate_array(gates_tensored, n);

            //Measured 1
            gates_tensored[control] = PROJ1;
            if(gate->type == CNOT) gates_tensored[target] = XGATE;
            if(gate->type == CZ) gates_tensored[target] = ZGATE;
            Matrix *gate_proj1 = get_tensored_gate_array(gates_tensored, n);

            matrix_add(gateMat, gate_proj1, &gateMat);

            matrix_free(gate_proj1);

            reset_gate_array(gates_tensored, n);
        }
        else if (gate->type == S0 || gate->type == ORACLE) {
            Matrix *temp;
            if(gate->type == S0) temp = get_S0_matrix(gate->params[1]);
            if(gate->type == ORACLE) temp = get_oracle_matrix(gate->params[1], gate->f);
            gateMat = get_tensored_gate_matrix(temp, target, n);
            matrix_free(temp);
        }
        else {
            Matrix *temp = matrix_of_array(2, 2, get_gate_matrix(get_corresponding_operator(gate->type, value)).matrix);
            gateMat = get_tensored_gate_matrix(temp, target, n);
            matrix_free(temp);
        }

        if(gate->type != MEAS || collapse) matrix_mult(gateMat, *statevector, statevector);
        
        matrix_free(gateMat);

        matrix_normalise(*statevector);
    }

    free(gates_tensored);

    return bits;
}
