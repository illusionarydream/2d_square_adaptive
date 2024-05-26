#include "octree.h"
#define LEARNING_RATE 0.01
#define ITERATIONS 50

int main() {
    Octree tree("../input/input_1.pgm");

    for (int i = 0; i < ITERATIONS; i++) {
        tree.grad_backwards();
        tree.grad_update(LEARNING_RATE);
        tree.split_tree(5);

        if (i % 100 == 0) {
            std::cout << "iteration: " << i << std::endl;
        }
    }

    // *write image
    tree.write_image("../output/output_1.ppm");

    return 0;
}