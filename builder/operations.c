#include "operations.h"

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../utils/utils.h"

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
    
    double complex *new_mat = calloc_custom(dim * dim, sizeof(double complex));
    assert(new_mat != NULL);
    
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
    
    if(node->is_zero) return false;
    if(node->is_identity) return true;

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
        return is_identity(node->data.operation.left_child) && is_identity(node->data.operation.right_child);
    }
    return false;
}
bool is_zero(Node *node) {
    if (node == NULL) return false;
    
    if(node->is_zero) return true;
    if(node->is_identity) return false;

    if (node->gt == LEAF) {
        uint64_t total = node->dim * node->dim;
        for (uint64_t i = 0; i < total; i++) {
            if (cabs(node->data.leaf.mat[i]) > 1e-15) return false;
        }
        return true;
    }
    if(node->is_zero) return true;
    if (node->gt == OP_TENSOR || node->gt == OP_PRODUCT) {
        return is_zero(node->data.operation.left_child) || is_zero(node->data.operation.right_child);
    }
    if (node->gt == OP_SUM) {
        return is_zero(node->data.operation.left_child) && is_zero(node->data.operation.right_child);
    }
    return false;
}
bool nodes_equal(Node *a, Node *b) {
    if (a == b) return true;
    if (a == NULL || b == NULL) return false;
    if (a->gt != b->gt || a->nb_qbits != b->nb_qbits || a->dim != b->dim) return false;

    if(a->is_identity && b->is_identity) return true;
    if(a->is_zero && b->is_zero) return true;

    if (a->gt == LEAF) {
        uint64_t total = a->dim * a->dim;
        for (uint64_t i = 0; i < total; i++) {
            if (cabs(a->data.leaf.mat[i] - b->data.leaf.mat[i]) > 1e-15) return false;
        }
        return true;
    }
    return nodes_equal(a->data.operation.left_child, b->data.operation.left_child) && 
            nodes_equal(a->data.operation.right_child, b->data.operation.right_child);
}

Node *simplify_nodes(Node *circuit) {
    if(circuit == NULL) return circuit;
    if(circuit->is_identity || circuit->is_zero) return circuit;

    if(circuit->gt == LEAF) {
        if(!circuit->is_zero && is_zero(circuit)) circuit->is_zero = true;
        else if(!circuit->is_identity && is_identity(circuit)) circuit->is_identity = true;
        return circuit;
    }
    
    circuit->data.operation.left_child = simplify_nodes(circuit->data.operation.left_child);
    circuit->data.operation.right_child = simplify_nodes(circuit->data.operation.right_child);

    if(!circuit->is_zero && is_zero(circuit)) {
        free_node(circuit->data.operation.left_child, true);
        free_node(circuit->data.operation.right_child, true);
        circuit->data.operation.left_child = NULL;
        circuit->data.operation.right_child = NULL;
        circuit->is_zero = true;
    }
    else if(!circuit->is_identity && is_identity(circuit)) {
        free_node(circuit->data.operation.left_child, true);
        free_node(circuit->data.operation.right_child, true);
        circuit->data.operation.left_child = NULL;
        circuit->data.operation.right_child = NULL;
        circuit->is_identity = true;
    }

    return circuit;
}
Node *product_fusion(Node *circuit) {
    if (circuit == NULL || circuit->gt == LEAF || circuit->is_identity || circuit->is_zero) return circuit;

    circuit->data.operation.left_child = product_fusion(circuit->data.operation.left_child);
    circuit->data.operation.right_child = product_fusion(circuit->data.operation.right_child);

    Node *left = circuit->data.operation.left_child;
    Node *right = circuit->data.operation.right_child;

    if (circuit->gt == OP_PRODUCT) {
        if(left->gt == LEAF && right->gt == LEAF && left->dim == right->dim) {
            Node *res = calc_product(left, right);
            free_node(circuit, true);
            return res;
        }
    }

    return circuit;
}
Node *sum_fusion(Node *circuit) {
    if (circuit == NULL || circuit->gt == LEAF || circuit->is_identity || circuit->is_zero) return circuit;

    circuit->data.operation.left_child = sum_fusion(circuit->data.operation.left_child);
    circuit->data.operation.right_child = sum_fusion(circuit->data.operation.right_child);

    Node *left = circuit->data.operation.left_child;
    Node *right = circuit->data.operation.right_child;

    if (circuit->gt == OP_SUM) {
        if(left->gt == LEAF && right->gt == LEAF && left->dim == right->dim) {
            Node *res = calc_sum(left, right);
            free_node(circuit, true);
            return res;
        }
    }

    return circuit;
}
Node *factorise_tensor(Node *circuit) {
    if(circuit == NULL || circuit->gt == LEAF || circuit->is_identity || circuit->is_zero) return circuit;

    Node *left = circuit->data.operation.left_child;
    Node *right = circuit->data.operation.right_child;

    if(circuit->gt == OP_PRODUCT && left->gt == OP_TENSOR && right->gt == OP_TENSOR && left->dim == right->dim) {
        //Checks in case other simplifications have not been applied
        if(left->is_zero || is_zero(left)) {
            free_node(left, true);
            free_node(right, true);
            circuit->is_zero = true;
            circuit->data.operation.left_child = NULL;
            circuit->data.operation.right_child = NULL;
            return circuit;
        }
        if(right->is_zero || is_zero(right)) {
            free_node(left, true);
            free_node(right, true);
            circuit->is_zero = true;
            circuit->data.operation.left_child = NULL;
            circuit->data.operation.right_child = NULL;
            return circuit;
        }
        if(left->is_identity || is_identity(left)) {
            free_node(left, false);
            free_node(circuit, false);
            return right;
        }
        if(right->is_identity || is_identity(right)) {
            free_node(right, false);
            free_node(circuit, false);
            return left;
        }

        Node *A = left->data.operation.left_child;
        Node *B = left->data.operation.right_child;
        Node *C = right->data.operation.left_child;
        Node *D = right->data.operation.right_child;
        if(A->dim != C->dim || B->dim != D->dim) return circuit;

        Node *new_left = create_product(A, C);
        Node *new_right = create_product(B, D);
        new_left = factorise_tensor(new_left);
        new_right = factorise_tensor(new_right);

        Node *new_node = create_tensor(new_left, new_right);

        free_node(left, false); free_node(right, false);
        free_node(circuit, false);

        return new_node;
    }
    else {
        circuit->data.operation.left_child = factorise_tensor(left);
        circuit->data.operation.right_child = factorise_tensor(right);
    }

    return circuit;
}

Node *distrib_sum(Node *circuit) {
    if (circuit == NULL || circuit->is_identity || circuit->is_zero || circuit->gt == LEAF) return circuit;

    Node *left = circuit->data.operation.left_child;
    Node *right = circuit->data.operation.right_child;

    if (circuit->gt == OP_PRODUCT) {
        if (right->gt == OP_SUM) {
            // A * (B + C) -> (A * B) + (A * C)
            Node *A = left;
            Node *B = right->data.operation.left_child;
            Node *C = right->data.operation.right_child;

            Node *new_left = create_product(copy_node(A), B);
            Node *new_right = create_product(A, C);

            circuit->gt = OP_SUM;
            circuit->data.operation.left_child = distrib_sum(new_left);
            circuit->data.operation.right_child = distrib_sum(new_right);
            
            free_node(right, false);
            return circuit;
        }
        else if (left->gt == OP_SUM) {
            // (A + B) * C -> (A * C) + (B * C)
            Node *A = left->data.operation.left_child;
            Node *B = left->data.operation.right_child;
            Node *C = right;

            Node *new_left = create_product(A, copy_node(C));
            Node *new_right = create_product(B, C);

            circuit->gt = OP_SUM;
            circuit->data.operation.left_child = distrib_sum(new_left);
            circuit->data.operation.right_child = distrib_sum(new_right);

            free_node(left, false);
            return circuit;
        }
    }

    circuit->data.operation.left_child = distrib_sum(left);
    circuit->data.operation.right_child = distrib_sum(right);

    return circuit;
}

Node *factorise_sum(Node *circuit) {
    if (circuit == NULL || circuit->gt == LEAF || circuit->is_identity || circuit->is_zero) return circuit;

    Node *left = circuit->data.operation.left_child;
    Node *right = circuit->data.operation.right_child;

    if (circuit->gt == OP_SUM && left->gt == OP_TENSOR && right->gt == OP_TENSOR) {
        Node *A = left->data.operation.left_child;
        Node *B = left->data.operation.right_child;
        Node *C = right->data.operation.left_child;
        Node *D = right->data.operation.right_child;

        // Case: (A ⊗ B) + (A ⊗ D) -> A ⊗ (B + D)
        if (nodes_equal(A, C)) {
            Node *new_sum = create_sum(B, D);
            Node *new_tensor = create_tensor(copy_node(A), new_sum);
            
            // Cleanup: free old SUM node and TENSOR nodes, but keep children that were reused or copied
            free_node(left, false); 
            free_node(right, false);
            free_node(circuit, false);
            
            return factorise_sum(new_tensor);
        }
        // Case: (A ⊗ B) + (C ⊗ B) -> (A + C) ⊗ B
        if (nodes_equal(B, D)) {
            Node *new_sum = create_sum(A, C);
            Node *new_tensor = create_tensor(new_sum, copy_node(B));

            free_node(left, false);
            free_node(right, false);
            free_node(circuit, false);

            return factorise_sum(new_tensor);
        }
    }

    circuit->data.operation.left_child = factorise_sum(left);
    circuit->data.operation.right_child = factorise_sum(right);

    return circuit;
}

Node *full_optimize(Node *circuit) {
    if (circuit == NULL) return NULL;

    Node *current = circuit;
    for(int i = 0; i < 5; i++) { // Increased iterations to allow propagation of simplifications
        current = factorise_tensor(current);
        current = factorise_sum(current);
        current = product_fusion(current);
        current = sum_fusion(current);
        current = simplify_nodes(current);
    }
    
    return current;
}