#ifndef VERTEX_H
#define VERTEX_H
#include <bits/stdc++.h>

struct vertex {
    double col;
    double grad_col;
    // the position of the vertex
    int x;
    int y;

    // initial
    vertex() {
        col = 0;
        grad_col = 0;
    }
    vertex(int _x, int _y) {
        x = _x;
        y = _y;
        col = 0;
        grad_col = 0;
    }
};
#endif