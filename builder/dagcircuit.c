#include "dagcircuit.h"

#include <stdlib.h>

typedef struct node_s {
    Gate* gate;
    struct node_s** children;
    int num_children;
} Node;

Node *node_create(Gate* gate, int max_children) {
    Node* node = malloc(sizeof(Node));
    node->gate = gate;
    node->children = calloc(max_children, sizeof(Node*));
    node->num_children = 0;
    return node;
}
void node_free(Node* node) {
    while (node->num_children > 0) {
        node_free(node->children[node->num_children - 1]);
        node->num_children--;
    }
    if (node->gate != NULL) free(node->gate);
    free(node->children);
    free(node);
}

DAGCircuit* dagcircuit_create(QuantumRegister* qreg, ClassicalRegister* creg) {
    DAGCircuit* dag = (DAGCircuit*)malloc(sizeof(DAGCircuit));
    dag->root = node_create(NULL, qreg->nb_qbits); // Create a root node with no gate
    dag->last_used = calloc(qreg->nb_qbits, sizeof(Node*));
    for(int i = 0; i < qreg->nb_qbits; i++) {
        dag->last_used[i] = dag->root;
    }
    dag->qreg = qreg;
    dag->creg = creg;
    return dag;
}
void dagcircuit_free(DAGCircuit* dag) {
    node_free(dag->root);
    free(dag->last_used);
    free(dag);
}

void dagcircuit_add_gate(DAGCircuit* dag, Gate* gate) {
    Node* new_node = node_create(gate, dag->qreg->nb_qbits);
    
    // Determine which qubits this gate acts on
    int* qubits = NULL;
    int nb_qubits = 0;
    switch (gate->class) {
        case MEAS:
            qubits = &(gate->gate.measure.qbit);
            nb_qubits = 1;
            break;
        case UNITARY:
            qubits = &(gate->gate.unitary.qbit);
            nb_qubits = 1;
            break;
        case CONTROL:
            qubits = malloc(2 * sizeof(int));
            qubits[0] = gate->gate.control.control;
            qubits[1] = gate->gate.control.qbit;
            nb_qubits = 2;
            break;
        case CUSTOM:
            qubits = gate->gate.custom.qbits;
            nb_qubits = gate->gate.custom.nb_qbits;
            break;
    }
    
    // Link the new node to the last used nodes for each qubit
    for (int i = 0; i < nb_qubits; i++) {
        int qbit = qubits[i];
        Node* last_node = dag->last_used[qbit];
        
        last_node->children[last_node->num_children] = new_node;
        last_node->num_children++;

        dag->last_used[qbit] = new_node;
    }

    if (gate->class == CONTROL) free(qubits);
}

void dagcircuit_add_unitary_gate(DAGCircuit *circuit, int t, SingleBitGate tg, double phase) {
    dagcircuit_add_gate(circuit, create_unitary_gate(t, tg, phase));
}
void dagcircuit_add_control_gate(DAGCircuit *circuit, int c, int t, SingleBitGate tg, double phase) {
    dagcircuit_add_gate(circuit, create_control_gate(c, t, tg, phase));
}
// Mat size must be 2^nb_qbits !
void dagcircuit_add_custom_gate(DAGCircuit *circuit, int nb_qbits, int *t, double complex *mat, char *label) {
    dagcircuit_add_gate(circuit, create_custom_gate(nb_qbits, t, mat, label));
}
void dagcircuit_add_measure(DAGCircuit *circuit, int qbit, int cbit) {
    dagcircuit_add_gate(circuit, create_measure(qbit, cbit));
}