#ifndef GRAD_IN_GPU_H
#define GRAD_IN_GPU_H

#include "node.h"

void grad_in_gpu(vertex *vertices, Node *nodes, float *gt_col, int vertice_size, int node_size, int gt_col_size, int width, int pixel_size, float learning_rate);
#endif