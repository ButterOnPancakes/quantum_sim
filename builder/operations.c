#include "operations.h"

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

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
            if (cabs(l_val) > 1e-15) { // Zero imprecision check
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
Node *product_fusion(Node *circuit) {
    if (circuit == NULL || circuit->gt == LEAF) return circuit;

    for (int i = 0; i < circuit->nb_children; i++) {
        circuit->data.operation.children[i] = product_fusion(circuit->data.operation.children[i]);
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

Node *full_optimize(Node *circuit) {
    if (circuit == NULL) return NULL;
    // 1. Cleanup
    circuit = flatten_tree(circuit);
    circuit = simplify_nodes(circuit);
    
    // 2. Algebraic simplifications
    circuit = factorise_tensor(circuit);
    circuit = product_fusion(circuit);
    circuit = sum_fusion(circuit);
    
    // 3. Cleanup
    circuit = simplify_nodes(circuit);
    circuit = flatten_tree(circuit);
    
    return circuit;
}