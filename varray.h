#ifndef VARRAY_H
#define VARRAY_H
#include "vertex.h"

class Varray {
   public:
    // data
    vertex *Vertex;
    int len;
    int size;

   public:
    // function
    Varray() {
        len = 0;
        size = 100;
        Vertex = new vertex[size];
    }
    // insert a vertex
    int insert(vertex v) {
        if (len == size) {
            vertex *newVertex = new vertex[size * 2];
            for (int i = 0; i < size; i++) {
                newVertex[i] = Vertex[i];
            }
            delete[] Vertex;
            Vertex = newVertex;
            size *= 2;
        }
        Vertex[len++] = v;
        return len - 1;
    }
    // operator [] edittalbe
    vertex &operator[](int index) {
        return Vertex[index];
    }
    // operator [] const
    const vertex &operator[](int index) const {
        return Vertex[index];
    }
};

extern Varray VertexArray;
#endif