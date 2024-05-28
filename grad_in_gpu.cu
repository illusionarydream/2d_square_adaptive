#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include "grad_in_gpu.cuh"

#define gt_col(y, x) gt_col[(y) * width + (x)]

__global__ void grad_update_for_one_thread(vertex *vertices, int size, float learning_rate) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) {
        // printf("idx: %d, col: %f, grad_col: %f\n", idx, vertices[idx].col, vertices[idx].grad_col);
        vertices[idx].col -= learning_rate * vertices[idx].grad_col;
        vertices[idx].col = min(1.0f, max(0.0f, vertices[idx].col));
        vertices[idx].grad_col = 0;
    }
}

// calculate the gradient of the color of the vertex
__global__ void grad_backwards_for_one_thread(vertex *vertices, Node *nodes, float *gt_col, int width, int size) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) {
        int ori_y = int(idx / width);
        int ori_x = idx % width;
        float y = int(idx / width) + 0.5;
        float x = idx % width + 0.5;

        // find the node that contains the pixel
        Node node = nodes[0];
        int node_index = 0;
        while (node.node_index[0] != -1) {  // if the node has subnodes

            float mid_x = (vertices[node.vertex_index[0]].x + vertices[node.vertex_index[2]].x) / 2.0;  // (x0 + x2) / 2
            float mid_y = (vertices[node.vertex_index[0]].y + vertices[node.vertex_index[2]].y) / 2.0;  // (y0 + y2) / 2

            if (y >= mid_y) {
                if (x >= mid_x) {  // 0-phase
                    node_index = node.node_index[0];
                    node = nodes[node.node_index[0]];
                } else {  // 1-phase
                    node_index = node.node_index[1];
                    node = nodes[node.node_index[1]];
                }
            } else {
                if (x >= mid_x) {  // 3-phase
                    node_index = node.node_index[3];
                    node = nodes[node.node_index[3]];
                } else {  // 2-phase
                    node_index = node.node_index[2];
                    node = nodes[node.node_index[2]];
                }
            }
        }

        // get the four vertices of the node
        int *vertex_idx = node.vertex_index;
        vertex rt = vertices[vertex_idx[0]];
        vertex lt = vertices[vertex_idx[1]];
        vertex lb = vertices[vertex_idx[2]];
        vertex rb = vertices[vertex_idx[3]];

        // bilinear interpolation
        float right = rt.x;
        float top = rt.y;
        float left = lb.x;
        float bottom = lb.y;

        float col_rt = rt.col;
        float col_lt = lt.col;
        float col_lb = lb.col;
        float col_rb = rb.col;

        float sum_inv = 1 / ((right - left) * (top - bottom));
        float w_rt = (x - left) * (y - bottom) * sum_inv;
        float w_lt = (right - x) * (y - bottom) * sum_inv;
        float w_lb = (right - x) * (top - y) * sum_inv;
        float w_rb = (x - left) * (top - y) * sum_inv;

        float w = w_lb * col_lb + w_lt * col_lt + w_rb * col_rb + w_rt * col_rt;

        // calculate the loss
        float c = gt_col(ori_y, ori_x);
        float grad = 2 * (w - c);
        float loss = grad * grad / 4;

        // if (x == 0 && y == 0)
        // printf("x: %d, y: %d, w: %f, c: %f, grad: %f, loss: %f\n", x, y, w, c, grad, loss);

        // calculate the gradient of the color of the four vertices
        atomicAdd(&vertices[vertex_idx[0]].grad_col, grad * w_rt);
        atomicAdd(&vertices[vertex_idx[1]].grad_col, grad * w_lt);
        atomicAdd(&vertices[vertex_idx[2]].grad_col, grad * w_lb);
        atomicAdd(&vertices[vertex_idx[3]].grad_col, grad * w_rb);

        // calculate the loss of the node
        atomicAdd(&nodes[node_index].loss_sum, loss);
    }
}

void grad_in_gpu(vertex *vertices,
                 Node *nodes,
                 float *gt_col,
                 int vertice_size,
                 int node_size,
                 int gt_col_size,
                 int width,
                 int pixel_size,
                 float learning_rate) {
    // std::cout << "vertices grad_col before: " << std::endl;
    // for (int i = 0; i < vertice_size; i++) {
    //     std::cout << vertices[i].col << ' ' << vertices[i].grad_col << std::endl;
    // }

    // std::cout << "nodes loss_sum before: " << std::endl;
    // for (int i = 0; i < node_size; i++) {
    //     std::cout << nodes[i].loss_sum << std::endl;
    // }
    // vertices data copy to GPU
    vertex *d_vertices;
    cudaMalloc(&d_vertices, vertice_size * sizeof(vertex));
    cudaMemcpy(d_vertices, vertices, vertice_size * sizeof(vertex), cudaMemcpyHostToDevice);

    // nodes data copy to GPU
    Node *d_nodes;
    cudaMalloc(&d_nodes, node_size * sizeof(Node));
    cudaMemcpy(d_nodes, nodes, node_size * sizeof(Node), cudaMemcpyHostToDevice);

    // gt_col data copy to GPU
    float *d_gt_col;
    cudaMalloc(&d_gt_col, gt_col_size * sizeof(float));
    cudaMemcpy(d_gt_col, gt_col, gt_col_size * sizeof(float), cudaMemcpyHostToDevice);

    // grad_backwards
    int back_block_size = 256;
    int back_grid_size = (pixel_size + back_block_size - 1) / back_block_size;
    grad_backwards_for_one_thread<<<back_grid_size, back_block_size>>>(d_vertices, d_nodes, d_gt_col, width, pixel_size);

    cudaDeviceSynchronize();

    // update
    int update_block_size = 256;
    int update_grid_size = (vertice_size + update_block_size - 1) / update_block_size;
    grad_update_for_one_thread<<<update_grid_size, update_block_size>>>(d_vertices, vertice_size, learning_rate);
    cudaDeviceSynchronize();

    // vertices data copy back to CPU
    cudaMemcpy(vertices, d_vertices, vertice_size * sizeof(vertex), cudaMemcpyDeviceToHost);
    cudaFree(d_vertices);

    // nodes data copy back to CPU
    cudaMemcpy(nodes, d_nodes, node_size * sizeof(Node), cudaMemcpyDeviceToHost);
    cudaFree(d_nodes);

    // gt_col data copy back to CPU
    cudaMemcpy(gt_col, d_gt_col, gt_col_size * sizeof(float), cudaMemcpyDeviceToHost);
    cudaFree(d_gt_col);

    // for (int i = 0; i < node_size; i++) {
    //     std::cout << nodes[i].loss_sum << std::endl;
    // }
}
