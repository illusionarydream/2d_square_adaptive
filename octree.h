#ifndef OCTREE_H
#define OCTREE_H
#include "picture_process.h"
#include "varray.h"
#include "node.h"
#include "grad_in_gpu.cuh"

Varray VertexArray;

class Octree {
   private:
    float loss_sum;
    Node *octree;
    int len;
    int size;
    int root;
    float *gt_col;  // the origin image
    int width;
    int height;

   public:
    // for reading and writing image
    void read_image(const char *filename) {
        read_grayscale_image(filename, gt_col, width, height);
        std::cout << "Image width: " << width << std::endl;
        std::cout << "Image height: " << height << std::endl;
    }
    void write_image(const char *filename) {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                gt_col(i, j) = get_col(j, i);
            }
        }
        write_grayscale_image(filename, gt_col, width, height);
    }
    int find_vertex_index(int x, int y) {
        // for (int i = 0; i < VertexArray.len; i++) {
        //     if (VertexArray[i].x == x && VertexArray[i].y == y) {
        //         return i;
        //     }
        // }
        std::queue<int> q;
        q.push(root);
        while (q.empty() == false) {
            int node_index = q.front();
            q.pop();
            // get the node
            Node *node = &octree[node_index];
            // if the node is the leaf node
            for (int i = 0; i < 4; i++) {
                if (VertexArray[node->vertex_index[i]].x == x && VertexArray[node->vertex_index[i]].y == y) {
                    return node->vertex_index[i];
                }
            }
            if (node->node_index[0] != -1) {
                for (int i = 0; i < 4; i++) {
                    int left = VertexArray[node->vertex_index[2]].x;
                    int right = VertexArray[node->vertex_index[0]].x;
                    int top = VertexArray[node->vertex_index[0]].y;
                    int bottom = VertexArray[node->vertex_index[2]].y;
                    if (x >= left && x <= right && y >= bottom && y <= top) {
                        q.push(node->node_index[i]);
                    }
                }
            }
        }
        return -1;
    }
    void extend_octree() {
        if (len <= size - 10) {
            return;
        }
        size *= 2;
        Node *new_octree = new Node[size];
        for (int i = 0; i < len; i++) {
            new_octree[i] = octree[i];
        }
        delete[] octree;
        octree = new_octree;
    }

   public:
    // initialize the octree
    Octree(const char *filename) {
        loss_sum = 0;
        read_image(filename);
        // insert the vertex
        root = 0;
        len = 1;
        size = 1000;
        octree = new Node[size];
        octree[root] = Node(std::make_pair(width, height), std::make_pair(0, 0));
    }
    // split the node into 4 subnodes
    // ----------------------------------------
    // lt | t | rt
    // ---------
    // l  | c | r
    // ---------
    // lb | b | rb
    void split_node(Node *node) {
        if (node->node_index[0] != -1) {
            return;
        }

        int right = VertexArray[node->vertex_index[0]].x;
        int top = VertexArray[node->vertex_index[0]].y;
        int left = VertexArray[node->vertex_index[2]].x;
        int bottom = VertexArray[node->vertex_index[2]].y;
        int mid_x = (left + right) >> 1;
        int mid_y = (top + bottom) >> 1;

        // std::cout << left << " " << right << " " << top << " " << bottom << std::endl;

        int idx_rt = node->vertex_index[0];
        int idx_lt = node->vertex_index[1];
        int idx_lb = node->vertex_index[2];
        int idx_rb = node->vertex_index[3];

        float col_c = std::get<4>(node->bilinearInterpolation(mid_x, mid_y));
        float col_t = std::get<4>(node->bilinearInterpolation(mid_x, top));
        float col_b = std::get<4>(node->bilinearInterpolation(mid_x, bottom));
        float col_l = std::get<4>(node->bilinearInterpolation(left, mid_y));
        float col_r = std::get<4>(node->bilinearInterpolation(right, mid_y));

        // create new 5 vertex
        // c
        int idx_c = VertexArray.insert(vertex(mid_x, mid_y));
        VertexArray[idx_c].col = col_c;
        // t
        int idx_t = find_vertex_index(mid_x, top);
        if (idx_t == -1) {
            idx_t = VertexArray.insert(vertex(mid_x, top));
        }
        VertexArray[idx_t].col = col_t;
        // b
        int idx_b = find_vertex_index(mid_x, bottom);
        if (idx_b == -1) {
            idx_b = VertexArray.insert(vertex(mid_x, bottom));
        }
        VertexArray[idx_b].col = col_b;
        // l
        int idx_l = find_vertex_index(left, mid_y);
        if (idx_l == -1) {
            idx_l = VertexArray.insert(vertex(left, mid_y));
        }
        VertexArray[idx_l].col = col_l;
        // r
        int idx_r = find_vertex_index(right, mid_y);
        if (idx_r == -1) {
            idx_r = VertexArray.insert(vertex(right, mid_y));
        }
        VertexArray[idx_r].col = col_r;

        // build index
        for (int i = 0; i < 4; i++)
            node->node_index[i] = len + i;

        // extend octree
        extend_octree();

        // std::cout << len << std::endl;
        // split the node
        octree[len++] = Node(idx_rt, idx_t, idx_c, idx_r);
        octree[len++] = Node(idx_t, idx_lt, idx_l, idx_c);
        octree[len++] = Node(idx_c, idx_l, idx_lb, idx_b);
        octree[len++] = Node(idx_r, idx_c, idx_b, idx_rb);
    }
    // find the node that contains the point (x, y)
    Node *find_node(float x, float y) {
        Node *node = &octree[root];
        while (node->node_index[0] != -1) {  // if the node has subnodes

            float mid_x = (VertexArray[node->vertex_index[0]].x + VertexArray[node->vertex_index[2]].x) / 2.0;
            float mid_y = (VertexArray[node->vertex_index[0]].y + VertexArray[node->vertex_index[2]].y) / 2.0;

            if (y >= mid_y) {
                if (x >= mid_x) {  // 0-phase
                    node = &octree[node->node_index[0]];
                } else {  // 1-phase
                    node = &octree[node->node_index[1]];
                }
            } else {
                if (x >= mid_x) {  // 3-phase
                    node = &octree[node->node_index[3]];
                } else {  // 2-phase
                    node = &octree[node->node_index[2]];
                }
            }
        }

        return node;
    }
    // get the color of the point (x, y)
    float get_col(int x, int y) {
        Node *node = find_node(x + 0.5, y + 0.5);
        float w_rt, w_lt, w_rb, w_lb, w;
        std::tie(w_rt, w_lt, w_rb, w_lb, w) = node->bilinearInterpolation(x + 0.5, y + 0.5);
        return w;
    }
    // split the tree if the loss of the node is larger than the threshold
    void split_tree(float threshold) {
        std::queue<int> q;
        q.push(root);
        while (!q.empty()) {
            int node_index = q.front();
            Node *node = &octree[node_index];
            q.pop();

            if (node->node_index[0] == -1) {
                // if the node is only one pixel
                int idx_rt = node->vertex_index[0];
                int idx_lb = node->vertex_index[2];

                int pixel_num = (VertexArray[idx_rt].x - VertexArray[idx_lb].x) * (VertexArray[idx_rt].y - VertexArray[idx_lb].y);
                if (pixel_num <= 1) {
                    continue;
                }
                // if the loss of the node is larger than the threshold
                if (node->loss_sum > threshold) {
                    node->loss_sum = 0;
                    split_node(node);
                }
            } else {
                for (int i = 0; i < 4; i++) {
                    q.push(node->node_index[i]);
                }
            }
        }

        // for (int i = 0; i < VertexArray.len; i++) {
        //     if (VertexArray[i].x == 512)
        //         std::cout << VertexArray[i].x << " " << VertexArray[i].y << ' ' << VertexArray[i].col << std::endl;
        // }
        loss_sum = 0;
    }
    // update the color of the vertex
    void grad_update_in_gpu(float learning_rate) {
        grad_in_gpu(VertexArray.Vertex, octree, gt_col, VertexArray.len, len, width * height, width, width * height, learning_rate);
    }
};

#endif