#include "tree.h"
#include "picture_process.h"
#define LEARNING_RATE 0.01
#define ITERATIONS 1000

int width;
int height;

void initial_vertex() {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            Vertex[i][j].col = 0;
            Vertex[i][j].grad_col = 0;
        }
    }
}

int main() {
    PictureProcess pictureProcess;
    pictureProcess.read_image("../input/input.pgm");

    height = pictureProcess.height;
    width = pictureProcess.width;

    Tree tree(0, height, 0, width);
    initial_vertex();

    for (int i = 0; i < ITERATIONS; i++) {
        for (int x = 0; x < height; x++) {
            for (int y = 0; y < width; y++) {
                tree.grad_backwards(x, y, pictureProcess.map[x][y]);
            }
        }
        tree.grad_update(LEARNING_RATE);
        tree.split_tree(10);
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            double col = tree.get_col(i, j);
            pictureProcess.map[i][j] = col;
        }
    }

    pictureProcess.write_image("../output/output.ppm");

    return 0;
}