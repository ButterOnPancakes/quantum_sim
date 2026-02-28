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
    
    uint64_t total_elements = node->dim * node->dim;
    node->data.leaf.mat = malloc(total_elements * sizeof(double complex));
    memcpy(node->data.leaf.mat, mat, total_elements * sizeof(double complex));
    
    return node;
}
Node *create_1q_leaf(double complex a11, double complex a12, double complex a21, double complex a22) {
    double complex mat[4] = {a11, a12, a21, a22};
    return create_leaf(mat, 1);
}

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

void print_tree(Node *node, int depth) {
    if (node == NULL) return;
    for (int i = 0; i < depth; i++) printf("  ");

    switch (node->gt) {
        case LEAF:
            printf("LEAF (qbits: %d, dim: %lu) [", node->nb_qbits, (unsigned long)node->dim);
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
