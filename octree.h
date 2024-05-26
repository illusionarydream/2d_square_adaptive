#ifndef OCTREE_H
#define OCTREE_H
#include "picture_process.h"
#include "varray.h"
// ----------------------------------------
//  1 | 0
// -------
//  2 | 3
// ----------------------------------------
struct Node {
    double loss_sum;

    // index for the vertex
    int vertex_index[4];

    // for subnodes
    Node *node_index[4];

    Node() {
        loss_sum = 0;
        for (int i = 0; i < 4; i++) {
            vertex_index[i] = -1;
            node_index[i] = nullptr;
        }
    }
    Node(int rt, int lt, int lb, int rb) {
        loss_sum = 0;
        vertex_index[0] = rt;
        vertex_index[1] = lt;
        vertex_index[2] = lb;
        vertex_index[3] = rb;

        node_index[0] = nullptr;
        node_index[1] = nullptr;
        node_index[2] = nullptr;
        node_index[3] = nullptr;
    }
    Node(std::pair<int, int> point_rt, std::pair<int, int> point_lb) {
        loss_sum = 0;
        for (int i = 0; i < 4; i++) {
            vertex_index[i] = VertexArray.insert(vertex());
            VertexArray[vertex_index[i]].x = (i == 0 || i == 3) ? point_rt.first : point_lb.first;
            VertexArray[vertex_index[i]].y = (i == 0 || i == 1) ? point_rt.second : point_lb.second;
        }

        node_index[0] = nullptr;
        node_index[1] = nullptr;
        node_index[2] = nullptr;
        node_index[3] = nullptr;
    }
    // output: w_rt, w_lt, w_rb, w_lb, w
    std::tuple<double, double, double, double, double> bilinearInterpolation(double x, double y) {
        double right = VertexArray[vertex_index[0]].x;
        double top = VertexArray[vertex_index[0]].y;
        double left = VertexArray[vertex_index[2]].x;
        double bottom = VertexArray[vertex_index[2]].y;

        double col_rt = VertexArray[vertex_index[0]].col;
        double col_lt = VertexArray[vertex_index[1]].col;
        double col_lb = VertexArray[vertex_index[2]].col;
        double col_rb = VertexArray[vertex_index[3]].col;

        double sum_inv = 1 / ((right - left) * (top - bottom));
        double w_rt = (x - left) * (y - bottom) * sum_inv;
        double w_lt = (right - x) * (y - bottom) * sum_inv;
        double w_lb = (right - x) * (top - y) * sum_inv;
        double w_rb = (x - left) * (top - y) * sum_inv;

        double w = w_lb * col_lb + w_lt * col_lt + w_rb * col_rb + w_rt * col_rt;

        return std::make_tuple(w_rt, w_lt, w_rb, w_lb, w);
    }
};
class Octree {
   private:
    double loss_sum;
    Node *root;
    double **map;  // the origin image
    int width;
    int height;

   public:
    // for reading and writing image
    void read_image(const char *filename) {
        read_grayscale_image(filename, map, width, height);
        std::cout << "Image width: " << width << std::endl;
        std::cout << "Image height: " << height << std::endl;
    }
    void write_image(const char *filename) {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                map[i][j] = get_col(j, i);
            }
        }
        write_grayscale_image(filename, map, width, height);
    }
    int find_vertex_index(int x, int y) {
        for (int i = 0; i < VertexArray.len; i++) {
            if (VertexArray[i].x == x && VertexArray[i].y == y) {
                return i;
            }
        }
        return -1;
    }

   public:
    // initialize the octree
    Octree(const char *filename) {
        loss_sum = 0;
        read_image(filename);
        // insert the vertex
        root = new Node(std::make_pair(width, height), std::make_pair(0, 0));
    }
    // split the node into 4 subnodes
    // ----------------------------------------
    // lt | t | rt
    // ---------
    // l  | c | r
    // ---------
    // lb | b | rb
    void split_node(Node *node) {
        if (node->node_index[0] != nullptr) {
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

        double col_c = std::get<4>(node->bilinearInterpolation(mid_x, mid_y));
        double col_t = std::get<4>(node->bilinearInterpolation(mid_x, top));
        double col_b = std::get<4>(node->bilinearInterpolation(mid_x, bottom));
        double col_l = std::get<4>(node->bilinearInterpolation(left, mid_y));
        double col_r = std::get<4>(node->bilinearInterpolation(right, mid_y));

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

        // split the node
        node->node_index[0] = new Node(idx_rt, idx_t, idx_c, idx_r);
        node->node_index[1] = new Node(idx_t, idx_lt, idx_l, idx_c);
        node->node_index[2] = new Node(idx_c, idx_l, idx_lb, idx_b);
        node->node_index[3] = new Node(idx_r, idx_c, idx_b, idx_rb);
    }
    // find the node that contains the point (x, y)
    Node *find_node(int x, int y) {
        Node *node = root;
        while (node->node_index[0] != nullptr) {  // if the node has subnodes

            int mid_x = (VertexArray[node->vertex_index[0]].x + VertexArray[node->vertex_index[2]].x) >> 1;
            int mid_y = (VertexArray[node->vertex_index[0]].y + VertexArray[node->vertex_index[2]].y) >> 1;

            if (y >= mid_y) {
                if (x >= mid_x) {  // 0-phase
                    node = node->node_index[0];
                } else {  // 1-phase
                    node = node->node_index[1];
                }
            } else {
                if (x >= mid_x) {  // 3-phase
                    node = node->node_index[3];
                } else {  // 2-phase
                    node = node->node_index[2];
                }
            }
        }

        return node;
    }
    // get the color of the point (x, y)
    double get_col(int x, int y) {
        Node *node = find_node(x, y);
        double w_rt, w_lt, w_rb, w_lb, w;
        std::tie(w_rt, w_lt, w_rb, w_lb, w) = node->bilinearInterpolation(x + 0.5, y + 0.5);
        return w;
    }
    // split the tree if the loss of the node is larger than the threshold
    void split_tree(double threshold) {
        std::queue<Node *> q;
        q.push(root);
        while (!q.empty()) {
            Node *node = q.front();
            q.pop();

            if (node->node_index[0] == nullptr) {
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
        std::cout << "loss sum:" << loss_sum << std::endl;
        loss_sum = 0;
    }
    // update the color of the vertex
    void grad_update(double learning_rate) {
        std::cout << VertexArray.len << std::endl;
        for (int i = 0; i < VertexArray.len; i++) {
            VertexArray[i].col += learning_rate * VertexArray[i].grad_col;

            // std::cout << VertexArray[i].x << " " << VertexArray[i].y << " " << VertexArray[i].col << VertexArray[i].grad_col << std::endl;

            if (VertexArray[i].col < 0) {
                VertexArray[i].col = 0;
            } else if (VertexArray[i].col > 1) {
                VertexArray[i].col = 1;
            }
            VertexArray[i].grad_col = 0;
        }
    }
    // calculate the gradient of the color of the vertex
    void grad_backwards_for_one_node(int x, int y, double c) {
        Node *node = find_node(x, y);
        double w[4], _w;
        std::tie(w[0], w[1], w[2], w[3], _w) = node->bilinearInterpolation(x + 0.5, y + 0.5);

        double grad_w = 2 * (c - _w);
        for (int i = 0; i < 4; i++) {
            int tmp_idx = node->vertex_index[i];
            VertexArray[tmp_idx].grad_col += grad_w * w[i];
        }
        node->loss_sum += grad_w * grad_w;
        loss_sum += grad_w * grad_w;
    }
    // calculate the gradient of the color of the vertex
    void grad_backwards() {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                grad_backwards_for_one_node(j, i, map[i][j]);
            }
        }
    }
};

#endif