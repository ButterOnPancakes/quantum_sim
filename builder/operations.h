#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "emms.h"

Node *product_fusion(Node *circuit);
Node *flatten_tree(Node *circuit);
Node *factorise_tensor(Node *circuit);
Node *full_optimize(Node *circuit);

#endif