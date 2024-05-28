#ifndef NODE_H
#define NODE_H
#include "vertex.h"
#include "varray.h"

// ----------------------------------------
//  1 | 0
// -------
//  2 | 3
// ----------------------------------------
struct Node {
    float loss_sum;

    // index for the vertex
    int vertex_index[4];

    // index for subnodes
    int node_index[4];

    Node() {
        loss_sum = 0;
        for (int i = 0; i < 4; i++) {
            vertex_index[i] = -1;
            node_index[i] = -1;
        }
    }
    Node(int rt, int lt, int lb, int rb) {
        loss_sum = 0;
        vertex_index[0] = rt;
        vertex_index[1] = lt;
        vertex_index[2] = lb;
        vertex_index[3] = rb;

        for (int i = 0; i < 4; i++) {
            node_index[i] = -1;
        }
    }
    Node(std::pair<int, int> point_rt, std::pair<int, int> point_lb) {
        loss_sum = 0;
        for (int i = 0; i < 4; i++) {
            vertex_index[i] = VertexArray.insert(vertex());
            VertexArray[vertex_index[i]].x = (i == 0 || i == 3) ? point_rt.first : point_lb.first;
            VertexArray[vertex_index[i]].y = (i == 0 || i == 1) ? point_rt.second : point_lb.second;
        }

        for (int i = 0; i < 4; i++) {
            node_index[i] = -1;
        }
    }
    // output: w_rt, w_lt, w_rb, w_lb, w
    std::tuple<float, float, float, float, float> bilinearInterpolation(float x, float y) {
        float right = VertexArray[vertex_index[0]].x;
        float top = VertexArray[vertex_index[0]].y;
        float left = VertexArray[vertex_index[2]].x;
        float bottom = VertexArray[vertex_index[2]].y;

        float col_rt = VertexArray[vertex_index[0]].col;
        float col_lt = VertexArray[vertex_index[1]].col;
        float col_lb = VertexArray[vertex_index[2]].col;
        float col_rb = VertexArray[vertex_index[3]].col;

        float sum_inv = 1 / ((right - left) * (top - bottom));
        float w_rt = (x - left) * (y - bottom) * sum_inv;
        float w_lt = (right - x) * (y - bottom) * sum_inv;
        float w_lb = (right - x) * (top - y) * sum_inv;
        float w_rb = (x - left) * (top - y) * sum_inv;

        float w = w_lb * col_lb + w_lt * col_lt + w_rb * col_rb + w_rt * col_rt;

        return std::make_tuple(w_rt, w_lt, w_rb, w_lb, w);
    }
};

#endif