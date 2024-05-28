#include "octree.h"
#define LEARNING_RATE 0.01
#define ITERATIONS 1000

int main() {
    Octree tree("../input/input_1.pgm");

    for (int i = 0; i < ITERATIONS; i++) {
        std::cout << "iteration: " << i << std::endl;

        tree.grad_update_in_gpu(LEARNING_RATE);
        tree.split_tree(5);

        if (i % 100 == 0) {
            std::cout << "iteration: " << i << std::endl;
        }
    }

    // *write image
    tree.write_image("../output/output_1.ppm");

    return 0;
}