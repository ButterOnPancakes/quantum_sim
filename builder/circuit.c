#include "circuit.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include "emms.h"
#include "operations.h"

Node *create_identity_gate() { return create_1q_leaf(1, 0, 0, 1); }
Node *create_x_gate() { return create_1q_leaf(0, 1, 1, 0); }
Node *create_y_gate() { return create_1q_leaf(0, -I, I, 0); }
Node *create_z_gate() { return create_1q_leaf(1, 0, 0, -1); }
Node *create_h_gate() {
    double s = 1.0/sqrt(2.0);
    return create_1q_leaf(s, s, s, -s);
}

Node *create_identity_tree(int nb_tensor) {
    assert(nb_tensor > 0);
    if(nb_tensor == 1) {
        return create_identity_gate();
    }
    else if(nb_tensor % 2 == 0) {
        Node *left = create_identity_tree(nb_tensor / 2);
        Node *right = create_identity_tree(nb_tensor / 2);
        return create_tensor(left, right);
    }
    else {
        int nb_left = (nb_tensor - 1) / 2;
        Node *left = create_identity_tree(nb_left);
        Node *right = create_identity_tree(nb_tensor - nb_left);
        return create_tensor(left, right);
    }
}
Node *create_gate_layer(Node* gate_node, int total_qbits, int start_index) {
    int qbits_before = start_index;
    int qbits_after = total_qbits - (start_index + gate_node->nb_qbits);

    Node *current_layer = gate_node;

    if(qbits_after > 0) {
        Node* id_after = create_identity_tree(qbits_after);
        current_layer = create_tensor(current_layer, id_after);
    }

    if(qbits_before > 0) {
        Node* id_before = create_identity_tree(qbits_before);
        current_layer = create_tensor(id_before, current_layer);
    }
    
    return current_layer;
}
Node *create_full_tensor_layer(Node **nodes, int n) {
    Node *res = nodes[0];
    for (int i = 1; i < n; i++) {
        res = create_tensor(res, nodes[i]);
    }
    return res;
}

QuantumCircuit *circuit_create(int nb_qbits) {
    QuantumCircuit *qc = malloc(sizeof(QuantumCircuit));
    qc->nb_qbits = nb_qbits;
    qc->root = create_identity_tree(nb_qbits);
    return qc;
}
void circuit_free(QuantumCircuit *qc) {
    free_node(qc->root, true);
    free(qc);
}

void circuit_optimize(QuantumCircuit *qc) {
    qc->root = full_optimize(qc->root);
}

void circuit_add_x_gate(QuantumCircuit *qc, int target) {
    Node *layer = create_gate_layer(create_x_gate(), qc->nb_qbits, target);
    Node *new_root = create_product(layer, qc->root);
    qc->root = new_root;
}
void circuit_add_y_gate(QuantumCircuit *qc, int target) {
    Node *layer = create_gate_layer(create_y_gate(), qc->nb_qbits, target);
    Node *new_root = create_product(layer, qc->root);
    qc->root = new_root;
}
void circuit_add_z_gate(QuantumCircuit *qc, int target) {
    Node *layer = create_gate_layer(create_z_gate(), qc->nb_qbits, target);
    Node *new_root = create_product(layer, qc->root);
    qc->root = new_root;
}
void circuit_add_h_gate(QuantumCircuit *qc, int target) {
    Node *layer = create_gate_layer(create_h_gate(), qc->nb_qbits, target);
    Node *new_root = create_product(layer, qc->root);
    qc->root = new_root;
}

Node *create_p0_gate() { return create_1q_leaf(1, 0, 0, 0); }
Node *create_p1_gate() { return create_1q_leaf(0, 0, 0, 1); }
Node *create_p01_gate() { return create_1q_leaf(0, 1, 0, 0); }
Node *create_p10_gate() { return create_1q_leaf(0, 0, 1, 0); }
void circuit_add_cnot_gate(QuantumCircuit *qc, int control, int target) {
    int n = qc->nb_qbits;
    Node **nodes_term0 = malloc(n * sizeof(Node*));
    Node **nodes_term1 = malloc(n * sizeof(Node*));

    for (int i = 0; i < n; i++) {
        if (i == control) {
            nodes_term0[i] = create_p0_gate();
            nodes_term1[i] = create_p1_gate();
        } else if (i == target) {
            nodes_term0[i] = create_identity_gate();
            nodes_term1[i] = create_x_gate();
        } else {
            nodes_term0[i] = create_identity_gate();
            nodes_term1[i] = create_identity_gate();
        }
    }

    Node *term0 = create_full_tensor_layer(nodes_term0, n);
    Node *term1 = create_full_tensor_layer(nodes_term1, n);
    
    free(nodes_term0);
    free(nodes_term1);

    Node *cnot_layer = create_sum(term0, term1);
    Node *new_root = create_product(cnot_layer, qc->root);
    qc->root = new_root;
}
void circuit_add_cz_gate(QuantumCircuit *qc, int control, int target) {
    int n = qc->nb_qbits;
    Node **nodes_term0 = malloc(n * sizeof(Node*));
    Node **nodes_term1 = malloc(n * sizeof(Node*));

    for (int i = 0; i < n; i++) {
        if (i == control) {
            nodes_term0[i] = create_p0_gate();
            nodes_term1[i] = create_p1_gate();
        } else if (i == target) {
            nodes_term0[i] = create_identity_gate();
            nodes_term1[i] = create_z_gate();
        } else {
            nodes_term0[i] = create_identity_gate();
            nodes_term1[i] = create_identity_gate();
        }
    }

    Node *term0 = create_full_tensor_layer(nodes_term0, n);
    Node *term1 = create_full_tensor_layer(nodes_term1, n);
    
    free(nodes_term0);
    free(nodes_term1);

    Node *cz_layer = create_sum(term0, term1);
    Node *new_root = create_product(cz_layer, qc->root);
    qc->root = new_root;
}
void circuit_add_cphase_gate(QuantumCircuit *qc, int control, int target, double theta) {
    int n = qc->nb_qbits;
    Node **nodes_term0 = malloc(n * sizeof(Node*));
    Node **nodes_term1 = malloc(n * sizeof(Node*));

    for (int i = 0; i < n; i++) {
        if (i == control) {
            nodes_term0[i] = create_p0_gate();
            nodes_term1[i] = create_p1_gate();
        } else if (i == target) {
            nodes_term0[i] = create_identity_gate();
            nodes_term1[i] = create_1q_leaf(cos(theta), -I*sin(theta), -I*sin(theta), cos(theta));
        } else {
            nodes_term0[i] = create_identity_gate();
            nodes_term1[i] = create_identity_gate();
        }
    }

    Node *term0 = create_full_tensor_layer(nodes_term0, n);
    Node *term1 = create_full_tensor_layer(nodes_term1, n);
    
    free(nodes_term0);
    free(nodes_term1);

    Node *cphase_layer = create_sum(term0, term1);
    Node *new_root = create_product(cphase_layer, qc->root);
    qc->root = new_root;
}
void circuit_add_swap_gate(QuantumCircuit *qc, int q1, int q2) {
    int n = qc->nb_qbits;
    
    Node **term00 = malloc(n * sizeof(Node*)); // |0><0| ⊗ |0><0|
    Node **term11 = malloc(n * sizeof(Node*)); // |1><1| ⊗ |1><1|
    Node **term01 = malloc(n * sizeof(Node*)); // |0><1| ⊗ |1><0|
    Node **term10 = malloc(n * sizeof(Node*)); // |1><0| ⊗ |0><1|

    for (int i = 0; i < n; i++) {
        if (i == q1) {
            term00[i] = create_p0_gate();   // |0><0|
            term11[i] = create_p1_gate();   // |1><1|
            term01[i] = create_p01_gate();  // |0><1|
            term10[i] = create_p10_gate();  // |1><0|
        } else if (i == q2) {
            term00[i] = create_p0_gate();   // |0><0|
            term11[i] = create_p1_gate();   // |1><1|
            term01[i] = create_p10_gate();  // |1><0|  <-- Swapped
            term10[i] = create_p01_gate();  // |0><1|  <-- Swapped
        } else {
            term00[i] = create_identity_gate();
            term11[i] = create_identity_gate();
            term01[i] = create_identity_gate();
            term10[i] = create_identity_gate();
        }
    }

    Node *t0 = create_full_tensor_layer(term00, n);
    Node *t1 = create_full_tensor_layer(term11, n);
    Node *t2 = create_full_tensor_layer(term01, n);
    Node *t3 = create_full_tensor_layer(term10, n);
    
    Node *sumA = create_sum(t0, t1);
    Node *sumB = create_sum(t2, t3);
    Node *swap_layer = create_sum(sumA, sumB);

    qc->root = create_product(swap_layer, qc->root);

    free(term00); free(term11); free(term01); free(term10);
}