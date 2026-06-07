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
    bool is_zero;
    bool is_identity;

    union {
        struct {
            float complex *mat;
        } leaf;

        struct {
            struct node_s *left_child;
            struct node_s *right_child;
        } operation;
    } data;
};
typedef struct node_s Node;

Node *create_leaf(float complex *mat, int nb_qbits);
Node *create_1q_leaf(float complex a11, float complex a12, float complex a21, float complex a22);
Node *create_gate_layer(Node* gate_node, int total_qbits, int start_index);

Node *create_sum(Node *left, Node *right);
Node *create_product(Node *left, Node *right);
Node *create_tensor(Node *left, Node *right);
void free_node(Node *node, bool recursive);
Node *copy_node(Node *node);

void print_tree(Node *node, int depth);
int count_nodes(Node *node);
int tree_depth(Node *node);

#endif