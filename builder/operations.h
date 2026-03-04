#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "emms.h"

Node *product_fusion(Node *circuit);
Node *sum_fusion(Node *circuit);
Node *factorise_tensor(Node *circuit);
Node *factorise_sum(Node *circuit);
Node *full_optimize(Node *circuit);

#endif