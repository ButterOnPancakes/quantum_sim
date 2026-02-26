#include "emms.h"

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

Node *create_leaf(double complex *mat, int nb_qbits) {
    Node *node = malloc(sizeof(Node));
    node->nb_qbits = nb_qbits;
    node->dim = 1ULL << nb_qbits;
    node->gt = LEAF;
    node->nb_children = 0;
    
    // Deep copy matrix
    uint64_t total_elements = node->dim * node->dim;
    node->data.leaf.mat = malloc(total_elements * sizeof(double complex));
    memcpy(node->data.leaf.mat, mat, total_elements * sizeof(double complex));
    
    return node;
}

Node *create_1q_leaf(double complex a11, double complex a12, double complex a21, double complex a22) {
    double complex mat[4] = {a11, a12, a21, a22};
    return create_leaf(mat, 1);
}

Node *create_identity_gate() { return create_1q_leaf(1, 0, 0, 1); }
Node *create_x_gate() { return create_1q_leaf(0, 1, 1, 0); }
Node *create_h_gate() { 
    double s = 1.0/sqrt(2.0);
    return create_1q_leaf(s, s, s, -s); 
}
Node *create_p0_gate() { return create_1q_leaf(1, 0, 0, 0); }
Node *create_p1_gate() { return create_1q_leaf(0, 0, 0, 1); }

Node *create_sum(Node *left, Node *right) {
    assert(left->dim == right->dim);
    Node *node = malloc(sizeof(Node));
    node->dim = left->dim;
    node->nb_qbits = left->nb_qbits;
    node->gt = OP_SUM;

    node->nb_children = 2;
    node->data.operation.children = malloc(node->nb_children * sizeof(Node*));
    node->data.operation.children[0] = left;
    node->data.operation.children[1] = right;
    return node;
}
Node *create_product(Node *left, Node *right) {
    assert(left->dim == right->dim);
    Node *node = malloc(sizeof(Node));
    node->dim = left->dim;
    node->nb_qbits = left->nb_qbits;
    node->gt = OP_PRODUCT;

    node->nb_children = 2;
    node->data.operation.children = malloc(node->nb_children * sizeof(Node*));
    node->data.operation.children[0] = left;
    node->data.operation.children[1] = right;
    return node;
}
Node *create_product_list(Node **nodes, int len) {
    assert(len > 0);
    Node *node = malloc(sizeof(Node));
    node->dim = nodes[0]->dim;
    node->nb_qbits = nodes[0]->nb_qbits;
    node->gt = OP_PRODUCT;

    node->nb_children = len;
    node->data.operation.children = nodes;
    return node;
}
Node *create_tensor(Node *left, Node *right) {
    Node *node = malloc(sizeof(Node));
    node->dim = left->dim * right->dim;
    node->nb_qbits = left->nb_qbits + right->nb_qbits;
    node->gt = OP_TENSOR;
    
    node->nb_children = 2;
    node->data.operation.children = malloc(node->nb_children * sizeof(Node*));
    node->data.operation.children[0] = left;
    node->data.operation.children[1] = right;
    return node;
}
Node *create_tensor_list(Node **nodes, int len) {
    Node *node = malloc(sizeof(Node));
    node->nb_qbits = 0;
    for(int i = 0; i < len; i++) {
        node->nb_qbits += nodes[i]->nb_qbits;
    }
    node->dim = 1ULL << node->nb_qbits;
    node->gt = OP_TENSOR;
    
    node->nb_children = len;
    node->data.operation.children = nodes;
    return node;
}
void free_node(Node *node, bool recursive) {
    if(node->gt == LEAF) {
        free(node->data.leaf.mat);
        free(node);
    }
    else {
        if(recursive) {
            for(int i = 0; i < node->nb_children; i++) {
                free_node(node->data.operation.children[i], recursive);
            }
        }
        free(node->data.operation.children);
        free(node);
    }
}

Node *copy_node(Node *node) {
    if (node == NULL) return NULL;
    Node *new_node = malloc(sizeof(Node));
    *new_node = *node; // shallow copy fields
    
    if (node->gt == LEAF) {
        uint64_t total = node->dim * node->dim;
        new_node->data.leaf.mat = malloc(total * sizeof(double complex));
        memcpy(new_node->data.leaf.mat, node->data.leaf.mat, total * sizeof(double complex));
    } else {
        new_node->data.operation.children = malloc(node->nb_children * sizeof(Node*));
        for (int i = 0; i < node->nb_children; i++) {
            new_node->data.operation.children[i] = copy_node(node->data.operation.children[i]);
        }
    }
    return new_node;
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

Node *create_circuit(int total_qbits) {
    return create_identity_tree(total_qbits);
}
Node *add_gate_to_circuit(Node* old_circuit, Node* gate, int target_qubit) {
    Node* layer = create_gate_layer(gate, old_circuit->nb_qbits, target_qubit);
    return create_product(layer, old_circuit);
}
Node *add_cnot_to_circuit(Node* old_circuit, int control, int target) {
    int n = old_circuit->nb_qbits;
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
    return create_product(cnot_layer, old_circuit);
}

void print_tree(Node *node, int depth) {
    if (node == NULL) return;
    for (int i = 0; i < depth; i++) printf("  ");

    switch (node->gt) {
        case LEAF:
            printf("LEAF (qbits: %d, dim: %lu) [", node->nb_qbits, (unsigned long)node->dim);
            // Print top-left 2x2 block
            if (node->dim >= 1) {
                printf("[%.2f+%.2fi...", creal(node->data.leaf.mat[0]), cimag(node->data.leaf.mat[0]));
            }
            printf("]\n");
            break;
        case OP_SUM:
            printf("SUM (qbits: %d, dim: %lu, children: %d)\n", node->nb_qbits, (unsigned long)node->dim, node->nb_children);
            break;
        case OP_PRODUCT:
            printf("PRODUCT (qbits: %d, dim: %lu, children: %d)\n", node->nb_qbits, (unsigned long)node->dim, node->nb_children);
            break;
        case OP_TENSOR:
            printf("TENSOR (qbits: %d, dim: %lu, children: %d)\n", node->nb_qbits, (unsigned long)node->dim, node->nb_children);
            break;
    }

    if (node->gt != LEAF) {
        for (int i = 0; i < node->nb_children; i++) {
            print_tree(node->data.operation.children[i], depth + 1);
        }
    }
}
int count_nodes(Node *node) {
    if (node == NULL) return 0;
    int count = 1;
    if (node->gt != LEAF) {
        for(int i = 0; i < node->nb_children; i++) {
            count += count_nodes(node->data.operation.children[i]);
        }
    }
    return count;
}
int tree_depth(Node *node) {
    if (node == NULL || node->gt == LEAF) return 1;
    int max_child_depth = 0;
    for (int i = 0; i < node->nb_children; i++) {
        int d = tree_depth(node->data.operation.children[i]);
        if (d > max_child_depth) max_child_depth = d;
    }
    return 1 + max_child_depth;
}

// Matrix Operations for Leaves
#define MAT_IDX(r, c, dim) ((r) * (dim) + (c))

Node *calc_sum(Node *left, Node *right) {
    assert(left->gt == LEAF && right->gt == LEAF);
    assert(left->dim == right->dim);
    
    Node *res = create_leaf(left->data.leaf.mat, left->nb_qbits); // Clone left
    uint64_t total = left->dim * left->dim;
    for (uint64_t i = 0; i < total; i++) {
        res->data.leaf.mat[i] += right->data.leaf.mat[i];
    }
    return res;
}
Node *calc_product(Node *left, Node *right) {
    assert(left->gt == LEAF && right->gt == LEAF);
    assert(left->dim == right->dim);
    uint64_t dim = left->dim;
    
    double complex *new_mat = calloc(dim * dim, sizeof(double complex));
    
    // Matrix Multiplication: res = left * right
    // Optimize for small matrices
    for (uint64_t i = 0; i < dim; i++) {
        for (uint64_t k = 0; k < dim; k++) {
            double complex l_val = left->data.leaf.mat[MAT_IDX(i, k, dim)];
            if (cabs(l_val) > 1e-15) { // Sparsity check
                for (uint64_t j = 0; j < dim; j++) {
                    new_mat[MAT_IDX(i, j, dim)] += l_val * right->data.leaf.mat[MAT_IDX(k, j, dim)];
                }
            }
        }
    }
    
    Node *res = create_leaf(new_mat, left->nb_qbits);
    free(new_mat);
    return res;
}

bool is_identity(Node *node) {
    if (node == NULL) return false;
    if (node->gt == LEAF) {
        uint64_t dim = node->dim;
        for (uint64_t i = 0; i < dim; i++) {
            for (uint64_t j = 0; j < dim; j++) {
                double complex val = node->data.leaf.mat[MAT_IDX(i, j, dim)];
                if (i == j) {
                    if (cabs(val - 1.0) > 1e-15) return false;
                } else {
                    if (cabs(val) > 1e-15) return false;
                }
            }
        }
        return true;
    }
    if (node->gt == OP_TENSOR || node->gt == OP_PRODUCT) {
        for (int i = 0; i < node->nb_children; i++) {
            if (!is_identity(node->data.operation.children[i])) return false;
        }
        return true;
    }
    return false;
}

bool is_zero(Node *node) {
    if (node == NULL) return false;
    if (node->gt == LEAF) {
        uint64_t total = node->dim * node->dim;
        for (uint64_t i = 0; i < total; i++) {
            if (cabs(node->data.leaf.mat[i]) > 1e-15) return false;
        }
        return true;
    }
    if (node->gt == OP_TENSOR || node->gt == OP_PRODUCT) {
        for (int i = 0; i < node->nb_children; i++) {
            if (is_zero(node->data.operation.children[i])) return true;
        }
    }
    if (node->gt == OP_SUM) {
        for (int i = 0; i < node->nb_children; i++) {
            if (!is_zero(node->data.operation.children[i])) return false;
        }
        return true;
    }
    return false;
}

bool nodes_equal(Node *a, Node *b) {
    if (a == b) return true;
    if (a == NULL || b == NULL) return false;
    if (a->gt != b->gt || a->nb_qbits != b->nb_qbits || a->dim != b->dim) return false;
    if (a->gt == LEAF) {
        uint64_t total = a->dim * a->dim;
        for (uint64_t i = 0; i < total; i++) {
            if (cabs(a->data.leaf.mat[i] - b->data.leaf.mat[i]) > 1e-15) return false;
        }
        return true;
    }
    if (a->nb_children != b->nb_children) return false;
    for (int i = 0; i < a->nb_children; i++) {
        if (!nodes_equal(a->data.operation.children[i], b->data.operation.children[i])) return false;
    }
    return true;
}

Node *flatten_tree(Node *circuit) {
    if(circuit->gt == LEAF) return circuit;

    for (int i = 0; i < circuit->nb_children; i++) {
        circuit->data.operation.children[i] = flatten_tree(circuit->data.operation.children[i]);
    }

    int nb_children = 0;
    for (int i = 0; i < circuit->nb_children; i++) {
        Node *child = circuit->data.operation.children[i];
        if (child->gt == circuit->gt) {
            nb_children += child->nb_children;
        } else {
            nb_children++;
        }
    }

    if (nb_children == circuit->nb_children) return circuit;
    
    Node **new_children = malloc(nb_children * sizeof(Node*));
    int wi = 0;
    for (int ri = 0; ri < circuit->nb_children; ri++) {
        Node *child = circuit->data.operation.children[ri];
        
        if (child->gt == circuit->gt) {
            for (int k = 0; k < child->nb_children; k++) {
                new_children[wi] = child->data.operation.children[k];
                wi++;
            }
            free_node(child, false); 
        } else {
            new_children[wi] = child;
            wi++;
        }
    }

    free(circuit->data.operation.children);
    circuit->nb_children = nb_children;
    circuit->data.operation.children = new_children;

    return circuit;
}

Node *simplify_nodes(Node *circuit) {
    if (circuit == NULL || circuit->gt == LEAF) return circuit;

    for (int i = 0; i < circuit->nb_children; i++) {
        circuit->data.operation.children[i] = simplify_nodes(circuit->data.operation.children[i]);
    }

    if (circuit->gt == OP_PRODUCT) {
        int i = 0;
        while (i < circuit->nb_children) {
            Node *child = circuit->data.operation.children[i];
            if (is_identity(child) && circuit->nb_children > 1) {
                for (int j = i; j < circuit->nb_children - 1; j++) {
                    circuit->data.operation.children[j] = circuit->data.operation.children[j + 1];
                }
                circuit->nb_children--;
                free_node(child, true);
                continue;
            }
            if (is_zero(child)) {
                for (int k = 0; k < circuit->nb_children; k++) {
                    if (k != i) free_node(circuit->data.operation.children[k], true);
                }
                Node *res = circuit->data.operation.children[i]; // Return the zero node
                free(circuit->data.operation.children);
                free(circuit);
                return res;
            }
            i++;
        }
    }
    
    if (circuit->gt == OP_SUM) {
        int i = 0;
        while (i < circuit->nb_children) {
            Node *child = circuit->data.operation.children[i];
            if (is_zero(child) && circuit->nb_children > 1) {
                for (int j = i; j < circuit->nb_children - 1; j++) {
                    circuit->data.operation.children[j] = circuit->data.operation.children[j + 1];
                }
                circuit->nb_children--;
                free_node(child, true);
                continue;
            }
            i++;
        }
    }

    if (circuit->nb_children == 1) {
        Node *child = circuit->data.operation.children[0];
        free(circuit->data.operation.children);
        free(circuit);
        return child;
    }

    return circuit;
}

Node *gate_fusion(Node *circuit) {
    if (circuit == NULL || circuit->gt == LEAF) return circuit;

    for (int i = 0; i < circuit->nb_children; i++) {
        circuit->data.operation.children[i] = gate_fusion(circuit->data.operation.children[i]);
    }

    if (circuit->gt == OP_PRODUCT) {
        int i = 0;
        while (i < circuit->nb_children - 1) {
            Node *curr = circuit->data.operation.children[i];
            Node *next = circuit->data.operation.children[i+1];

            if (curr->gt == LEAF && next->gt == LEAF && curr->nb_qbits == next->nb_qbits) {
                Node *fused = calc_product(curr, next);
                
                circuit->data.operation.children[i] = fused;
                for (int j = i + 1; j < circuit->nb_children - 1; j++) {
                    circuit->data.operation.children[j] = circuit->data.operation.children[j+1];
                }
                
                circuit->nb_children--;
                free_node(curr, true);
                free_node(next, true);
                continue; 
            }
            i++;
        }
    }

    if (circuit->nb_children == 1) {
        Node *child = circuit->data.operation.children[0];
        free(circuit->data.operation.children);
        free(circuit);
        return child;
    }

    return circuit;
}

Node *sum_fusion(Node *circuit) {
    if (circuit == NULL || circuit->gt == LEAF) return circuit;

    for (int i = 0; i < circuit->nb_children; i++) {
        circuit->data.operation.children[i] = sum_fusion(circuit->data.operation.children[i]);
    }

    if (circuit->gt == OP_SUM) {
        int i = 0;
        while (i < circuit->nb_children - 1) {
            Node *curr = circuit->data.operation.children[i];
            Node *next = circuit->data.operation.children[i + 1];

            if (curr->gt == LEAF && next->gt == LEAF && curr->nb_qbits == next->nb_qbits) {
                Node *fused = calc_sum(curr, next);
                circuit->data.operation.children[i] = fused;
                for (int j = i + 1; j < circuit->nb_children - 1; j++) {
                    circuit->data.operation.children[j] = circuit->data.operation.children[j + 1];
                }
                circuit->nb_children--;
                free_node(curr, true);
                free_node(next, true);
                continue;
            }
            i++;
        }
    }

    if (circuit->nb_children == 1) {
        Node *child = circuit->data.operation.children[0];
        free(circuit->data.operation.children);
        free(circuit);
        return child;
    }

    return circuit;
}

Node *factorise_tensor(Node *circuit) {
    if (circuit == NULL || circuit->gt == LEAF) return circuit;

    for (int i = 0; i < circuit->nb_children; i++) {
        circuit->data.operation.children[i] = factorise_tensor(circuit->data.operation.children[i]);
    }

    if (circuit->gt == OP_PRODUCT) {
        int amount = circuit->data.operation.children[0]->nb_children;
        for(int i = 0; i < circuit->nb_children; i++) {
            if(
                circuit->data.operation.children[i]->gt != OP_TENSOR || 
                circuit->data.operation.children[i]->nb_children != amount
            ) return circuit;
        }

        Node **nodes = malloc(amount * sizeof(Node *));
        for(int i = 0; i < amount; i++) {
            Node **node_list = malloc(circuit->nb_children * sizeof(Node*));
            for(int j = 0; j < circuit->nb_children; j++) {
                node_list[j] = circuit->data.operation.children[j]->data.operation.children[i];
            }
            nodes[i] = create_product_list(node_list, circuit->nb_children);
        }
        Node *newnode = create_tensor_list(nodes, amount);

        for(int i = 0; i < circuit->nb_children; i++) {
            free_node(circuit->data.operation.children[i], false);
        }
        free_node(circuit, false);
        return newnode;
    }

    return circuit;
}

// REMOVED distrib_sum to prevent explosion on dense/random circuits.
// REMOVED factorise_sum for simplicity (less critical than avoiding explosion).

Node *distrib_sum(Node *circuit) {
    // Placeholder: NO-OP to keep linker happy if needed, or just remove.
    // User requested structure change to prevent slowdown.
    return circuit;
}

Node *full_optimize(Node *circuit) {
    if (circuit == NULL) return NULL;
    // Iterative optimization
    // 1. Structural cleanup
    circuit = flatten_tree(circuit);
    circuit = simplify_nodes(circuit);
    
    // 2. Algebraic simplifications
    circuit = factorise_tensor(circuit);
    circuit = gate_fusion(circuit); // Fuses products of leaves
    circuit = sum_fusion(circuit);  // Fuses sums of leaves
    
    // 3. Final cleanup
    circuit = simplify_nodes(circuit);
    circuit = flatten_tree(circuit);
    
    return circuit;
}