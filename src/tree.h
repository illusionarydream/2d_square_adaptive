#ifndef TREE_H
#define TREE_
#include "vertex.h"

struct Node {
    int loss_sum;
    int left;
    int right;
    int top;
    int bottom;
    Node *left_bottom;
    Node *left_top;
    Node *right_bottom;
    Node *right_top;

    Node(int left, int right, int top, int bottom) {
        this->loss_sum = 0;
        this->left = left;
        this->right = right;
        this->top = top;
        this->bottom = bottom;
        this->left_bottom = nullptr;
        this->left_top = nullptr;
        this->right_bottom = nullptr;
        this->right_top = nullptr;
    }
    // output: w_lb, w_lt, w_rb, w_rt, w
    std::tuple<double, double, double, double, double> bilinearInterpolation(double x, double y) {
        double sum = (right - left) * (bottom - top);
        double w_lb = (right - x) * (bottom - y) / sum;
        double w_lt = (right - x) * (y - top) / sum;
        double w_rb = (x - left) * (bottom - y) / sum;
        double w_rt = (x - left) * (y - top) / sum;
        double w = w_lb * Vertex[left][top].col + w_lt * Vertex[left][bottom].col + w_rb * Vertex[right][top].col + w_rt * Vertex[right][bottom].col;
        return std::make_tuple(w_lb, w_lt, w_rb, w_rt, w);
    }
};
class Tree {
   private:
    Node *root;

   public:
    Tree(int left, int right, int top, int bottom) {
        root = new Node(left, right, top, bottom);
    }
    void split_node(Node *node) {
        int left = node->left;
        int right = node->right;
        int top = node->top;
        int bottom = node->bottom;
        int mid_x = (left + right) / 2;
        int mid_y = (top + bottom) / 2;
        node->left_bottom = new Node(left, mid_x, top, mid_y);
        node->left_top = new Node(left, mid_x, mid_y, bottom);
        node->right_bottom = new Node(mid_x, right, top, mid_y);
        node->right_top = new Node(mid_x, right, mid_y, bottom);
    }
    Node *find_node(int x, int y) {
        Node *node = root;
        while (node->left_bottom != nullptr) {
            int mid_x = (node->left + node->right) / 2;
            int mid_y = (node->top + node->bottom) / 2;
            if (x < mid_x) {
                if (y < mid_y) {
                    node = node->left_bottom;
                } else {
                    node = node->left_top;
                }
            } else {
                if (y < mid_y) {
                    node = node->right_bottom;
                } else {
                    node = node->right_top;
                }
            }
        }
        return node;
    }
    double get_col(int x, int y) {
        Node *node = find_node(x, y);
        double w_lb, w_lt, w_rb, w_rt, w;
        std::tie(w_lb, w_lt, w_rb, w_rt, w) = node->bilinearInterpolation(x, y);
        return w;
    }
    void split_tree(double threshold) {
        std::queue<Node *> q;
        q.push(root);
        while (!q.empty()) {
            Node *node = q.front();
            q.pop();
            if (node->left_bottom == nullptr) {
                if (node->loss_sum > threshold) {
                    node->loss_sum = 0;
                    split_node(node);
                }

            } else {
                q.push(node->left_bottom);
                q.push(node->left_top);
                q.push(node->right_bottom);
                q.push(node->right_top);
            }
        }
    }
    void grad_update(double learning_rate) {
        std::queue<Node *> q;
        q.push(root);
        while (!q.empty()) {
            Node *node = q.front();
            q.pop();
            if (node->left_bottom == nullptr) {
                int left = node->left;
                int right = node->right;
                int top = node->top;
                int bottom = node->bottom;

                Vertex[left][top].col += learning_rate * Vertex[left][top].grad_col;
                Vertex[left][bottom].col += learning_rate * Vertex[left][bottom].grad_col;
                Vertex[right][top].col += learning_rate * Vertex[right][top].grad_col;
                Vertex[right][bottom].col += learning_rate * Vertex[right][bottom].grad_col;

                Vertex[left][top].grad_col = 0;
                Vertex[left][bottom].grad_col = 0;
                Vertex[right][top].grad_col = 0;
                Vertex[right][bottom].grad_col = 0;
            } else {
                q.push(node->left_bottom);
                q.push(node->left_top);
                q.push(node->right_bottom);
                q.push(node->right_top);
            }
        }
    }
    void grad_backwards(double x, double y, double c) {
        Node *node = find_node(x, y);
        double w_lb, w_lt, w_rb, w_rt, w;
        std::tie(w_lb, w_lt, w_rb, w_rt, w) = node->bilinearInterpolation(x, y);
        double grad_w = 2 * (c - w);
        Vertex[node->left][node->top].grad_col += grad_w * w_lb;
        Vertex[node->left][node->bottom].grad_col += grad_w * w_lt;
        Vertex[node->right][node->top].grad_col += grad_w * w_rb;
        Vertex[node->right][node->bottom].grad_col += grad_w * w_rt;
        node->loss_sum += grad_w * grad_w;
    }
};

#endif