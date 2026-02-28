#ifndef EMMS_H
#define EMMS_H

#include <complex.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    LEAF,
    OP_SUM,
    OP_PRODUCT,
    OP_TENSOR
} GateType;

struct node_s {
    GateType gt;
    int nb_qbits;
    uint64_t dim;
    int nb_children;
    union {
        struct {
            double complex *mat;
        } leaf;

        struct {
            struct node_s **children;
        } operation;
    } data;
};
typedef struct node_s Node;

Node *create_leaf(double complex *mat, int nb_qbits);
Node *create_1q_leaf(double complex a11, double complex a12, double complex a21, double complex a22);
Node *create_gate_layer(Node* gate_node, int total_qbits, int start_index);

Node *create_sum(Node *left, Node *right);
Node *create_product(Node *left, Node *right);
Node *create_product_list(Node **nodes, int len);
Node *create_tensor(Node *left, Node *right);
Node *create_tensor_list(Node **nodes, int len);
void free_node(Node *node, bool recursive);
Node *copy_node(Node *node);

void print_tree(Node *node, int depth);
int count_nodes(Node *node);
int tree_depth(Node *node);

#endif