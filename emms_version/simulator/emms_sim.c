#include "emms_sim.h"
#include <stdlib.h>
#include <string.h>
#include <complex.h>

/**
 * Leaf Operator Application: y = M * x
 * Maps to: out[i] = sum_j (mat[i,j] * in[j])
 */
static void apply_leaf_op(uint64_t dim, const double complex *mat, 
                          BlockView in, BlockView out, BlockView scratch,
                          bool accumulate) {
    BlockView src = in;
    
    // Alias protection: If in and out overlap, copy in to scratch first
    if (in.data == out.data) {
        for (uint64_t i = 0; i < dim; i++) scratch.data[i * scratch.stride] = in.data[i * in.stride];
        src = scratch;
    }

    // Specialized 1-qubit gate (2x2 matrix)
    if (dim == 2) {
        double complex x0 = src.data[0];
        double complex x1 = src.data[src.stride];
        
        double complex y0 = mat[0] * x0 + mat[1] * x1;
        double complex y1 = mat[2] * x0 + mat[3] * x1;

        if (accumulate) {
            out.data[0] += y0;
            out.data[out.stride] += y1;
        } else {
            out.data[0] = y0;
            out.data[out.stride] = y1;
        }
        return;
    }

    // General Matrix-Vector Multiplication
    for (uint64_t i = 0; i < dim; i++) {
        double complex sum = 0;
        for (uint64_t j = 0; j < dim; j++) {
            sum += mat[i * dim + j] * src.data[j * src.stride];
        }
        if (accumulate) out.data[i * out.stride] += sum;
        else out.data[i * out.stride] = sum;
    }
}

/**
 * Recursive Operator Application
 * Handles the algebraic structure of the Quantum Circuit.
 */
static void apply_recursive(Node *op, BlockView in, BlockView out, 
                            BlockView aux1, BlockView aux2, bool accumulate) {
    uint64_t dim = op->dim;

    switch (op->gt) {
        case LEAF:
            apply_leaf_op(dim, op->data.leaf.mat, in, out, aux1, accumulate);
            break;

        case OP_SUM:
            // Linear Combination: Out = sum( Op_i * In )
            if (!accumulate) {
                for (uint64_t k = 0; k < dim; k++) out.data[k * out.stride] = 0;
            }
            for (int i = 0; i < op->nb_children; i++) {
                apply_recursive(op->data.operation.children[i], in, out, aux1, aux2, true);
            }
            break;

        case OP_PRODUCT: {
            // Sequential Composition: Out = (Op_n * ... * Op_1) * In
            if (op->nb_children == 0) {
                for (uint64_t k = 0; k < dim; k++) {
                    double complex val = in.data[k * in.stride];
                    if (accumulate) out.data[k * out.stride] += val; else out.data[k * out.stride] = val;
                }
                return;
            }

            BlockView current_state = in;
            BlockView bufA = aux1, bufB = aux2;

            for (int i = op->nb_children - 1; i >= 0; i--) {
                bool is_last = (i == 0);
                Node *child = op->data.operation.children[i];
                
                if (is_last) {
                    apply_recursive(child, current_state, out, bufA, bufB, accumulate);
                } else {
                    // Intermediate steps overwrite buffer
                    apply_recursive(child, current_state, bufA, bufB, bufB, false);
                    current_state = bufA;
                    // Ping-pong buffers for next iteration
                    BlockView tmp = bufA; bufA = bufB; bufB = tmp;
                }
            }
            break;
        }

        case OP_TENSOR: {
            // Kronecker Product Application: (A ⊗ B) * Psi
            // Decomposed as: (A ⊗ I) * (I ⊗ B) * Psi
            int n = op->nb_children;
            uint64_t *child_strides = malloc(n * sizeof(uint64_t));
            uint64_t accumulated_stride = 1;
            for (int i = n - 1; i >= 0; i--) {
                child_strides[i] = accumulated_stride;
                accumulated_stride *= op->data.operation.children[i]->dim;
            }

            BlockView current_state = in;
            BlockView bufA = aux1, bufB = aux2;

            for (int i = n - 1; i >= 0; i--) {
                bool is_last_gate = (i == 0);
                Node *child = op->data.operation.children[i];
                
                uint64_t c_dim = child->dim;
                uint64_t c_stride = child_strides[i];
                
                // Subspace dimensions
                uint64_t outer_dim = dim / (c_dim * c_stride);
                uint64_t inner_dim = c_stride;

                BlockView next_target = is_last_gate ? out : bufA;
                bool next_acc = is_last_gate ? accumulate : false;

                // Apply child operator to every subspace fiber
                for (uint64_t o = 0; o < outer_dim; o++) {
                    for (uint64_t k = 0; k < inner_dim; k++) {
                        uint64_t base_offset = o * c_dim * c_stride + k;
                        
                        // Map local subspace to global memory addresses
                        BlockView sub_in  = { current_state.data + base_offset * current_state.stride, current_state.stride * c_stride };
                        BlockView sub_out = { next_target.data + base_offset * next_target.stride, next_target.stride * c_stride };
                        
                        // Scratches must also preserve subspace strides
                        BlockView sa = { bufB.data + base_offset * bufB.stride, bufB.stride * c_stride };
                        BlockView sb = { bufA.data + base_offset * bufA.stride, bufA.stride * c_stride };

                        // Mathematical edge case: if we are writing to final output, check for buffer aliasing
                        if (!is_last_gate && !accumulate) {
                            sb = (BlockView){ out.data + base_offset * out.stride, out.stride * c_stride };
                        } else if (is_last_gate && current_state.data == bufA.data) {
                            sb = sa;
                        }

                        apply_recursive(child, sub_in, sub_out, sa, sb, next_acc);
                    }
                }

                if (!is_last_gate) {
                    current_state = bufA;
                    BlockView tmp = bufA; bufA = bufB; bufB = tmp;
                }
            }
            free(child_strides);
            break;
        }
    }
}

void emms_apply_inplace(Node *node, BlockView in, BlockView out, BlockView aux, bool acc) {
    // Split the double-sized aux buffer into two working spaces for the ping-pong logic
    BlockView aux1 = { aux.data, aux.stride };
    BlockView aux2 = { aux.data + node->dim, aux.stride };
    apply_recursive(node, in, out, aux1, aux2, acc);
}

double complex *emms_compute_statevector(Node *circuit) {
    uint64_t dim = circuit->dim;
    double complex *psi_init = calloc(dim, sizeof(double complex));
    psi_init[0] = 1.0; // Assume |0...0>
    
    double complex *psi_final = malloc(dim * sizeof(double complex));
    double complex *workspace = malloc(2 * dim * sizeof(double complex));
    
    BlockView bv_in  = { psi_init, 1 };
    BlockView bv_out = { psi_final, 1 };
    BlockView bv_aux = { workspace, 1 };
    
    emms_apply_inplace(circuit, bv_in, bv_out, bv_aux, false);
    
    free(psi_init);
    free(workspace);
    return psi_final;
}
