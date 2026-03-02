#include "emms_sim.h"

#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <assert.h>

/*
Representation of a reshaped column vector : See proof for more details
Use : vector[k] = data[k * offset_factor]
*/ 
typedef struct {
    uint64_t data_size; //Malloc-ed size
    uint64_t vector_size; //Vector size
    double complex *data;
    uint64_t offset_factor;
} Split;

Split copy_split(Split split) {
    Split s;
    s.data_size = split.data_size;
    s.vector_size = split.vector_size;
    s.offset_factor = split.offset_factor;
    s.data = malloc(split.data_size * sizeof(double complex));
    for(uint64_t i = 0; i < split.data_size; i++) s.data[i] = split.data[i];
    return s;
}
void free_split(Split split) {
    free(split.data);
}

void apply_node(Node *node, Split vector, Split output);

void apply_leaf(Node *node, Split vector, Split output) {
    assert(node != NULL && node->gt == LEAF && node->dim == vector.vector_size);
    assert(vector.data != output.data);

    double complex *mat = node->data.leaf.mat; //mat[i * size + k]
    for(uint64_t i = 0; i < vector.vector_size; i++) {
        output.data[i * output.offset_factor] = 0;
        for(uint64_t k = 0; k < node->dim; k++) {
            output.data[i * output.offset_factor] += mat[i * node->dim + k] * vector.data[k * vector.offset_factor];
        }
    }
}

void apply_sum(Node *node, Split input, Split output) {
    assert(node != NULL && node->gt == OP_SUM);

    Split temp = copy_split(output);

    apply_node(node->data.operation.left_child, input, output);
    apply_node(node->data.operation.right_child, input, temp);

    for(uint64_t i = 0; i < output.vector_size; i++) output.data[i * output.offset_factor] += temp.data[i * temp.offset_factor];

    free_split(temp);
}

void apply_product(Node *node, Split input, Split output) {
    assert(node != NULL && node->gt == OP_PRODUCT);

    Split temp = copy_split(output);

    apply_node(node->data.operation.right_child, input, temp);
    apply_node(node->data.operation.left_child, temp, output);

    free_split(temp);
}

void apply_tensor(Node *node, Split input, Split output) {
    assert(node != NULL && node->gt == OP_TENSOR);

    Split temp = copy_split(output);
    
    Node *A = node->data.operation.left_child;
    Node *B = node->data.operation.right_child;

    uint64_t A_dim = A->dim;
    uint64_t B_dim = B->dim;

    // Revient à calculer M' = B * Mk(phi) (chaque colonne C' <- B * C)
    for(uint64_t col = 0; col < A_dim; col++) {
        Split col_in = {
            .data = &input.data[col * B_dim * input.offset_factor],
            .offset_factor = input.offset_factor,
            .vector_size = B_dim,
            .data_size = B_dim * input.offset_factor //Change the size to avoid segfaults bc of wrong mem access
        };
        Split col_out = {
            .data = &temp.data[col * B_dim * temp.offset_factor],
            .offset_factor = temp.offset_factor,
            .vector_size = B_dim,
            .data_size = B_dim * temp.offset_factor //Change the size to avoid segfaults bc of wrong mem access
        };
        apply_node(B, col_in, col_out);
    }

    //Calcul de M'' = M' * A^T (chaque ligne L' <- L * A^T soit L'^T <- A * L^T)
    for(uint64_t row = 0; row < B_dim; row++) {
        Split row_in = {
            .data = &temp.data[row * temp.offset_factor],
            .offset_factor = B_dim * temp.offset_factor,
            .vector_size = A_dim,
            .data_size = temp.data_size - row * temp.offset_factor
        };
        Split row_out = {
            .data = &output.data[row * output.offset_factor],
            .offset_factor = B_dim * output.offset_factor,
            .vector_size = A_dim,
            .data_size = output.data_size - row * output.offset_factor
        };
        apply_node(A, row_in, row_out);
    }

    free_split(temp);
}

void apply_node(Node *node, Split vector, Split output) {
    assert(node != NULL);
    assert(vector.data != output.data && vector.vector_size == output.vector_size);
    assert(vector.vector_size == node->dim);
    if(node->is_zero) {
        for(uint64_t i = 0; i < output.vector_size; i++) output.data[i * output.offset_factor] = 0;
        return;
    }
    if(node->is_identity) {
        for(uint64_t i = 0; i < output.vector_size; i++) output.data[i * output.offset_factor] = vector.data[i * vector.offset_factor];
        return;
    }

    switch(node->gt) {
        case LEAF: apply_leaf(node, vector, output); break;
        case OP_SUM: apply_sum(node, vector, output); break;
        case OP_PRODUCT: apply_product(node, vector, output); break;
        case OP_TENSOR: apply_tensor(node, vector, output); break;
    }
}

void emms_compute_statevector(QuantumCircuit *circuit, double complex* vector, uint64_t dim) {
    assert(dim == (uint64_t) (1 << circuit->nb_qbits));
    Split output = {
        .data_size = dim,
        .vector_size = dim,
        .data = vector,
        .offset_factor = 1
    };
    Split input = copy_split(output);
    //Split temp = copy_split(input);

    apply_node(circuit->root, input, output);

    free_split(input);
}