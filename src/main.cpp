#include "tree.h"
#include <opencv2/opencv.hpp>
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
    cv::Mat image = cv::imread("../input/input.pgm", cv::IMREAD_GRAYSCALE);
    image.convertTo(image, CV_64F, 1.0 / 255.0);
    if (image.empty()) {
        std::cout << "Failed to read image" << std::endl;
        return -1;
    }

    width = image.cols;
    height = image.rows;


    Tree tree(0, height, 0, width);
    initial_vertex();
    
    printf("Start training\n");
    for (int i = 0; i < ITERATIONS; i++) {
        if (i % 100 == 0)
            printf("Iteration %d\n", i);
        

        for (int x = 0; x < height; x++) {
            for (int y = 0; y < width; y++) {
                tree.grad_backwards(x, y, image.at<double>(x, y));
            }
        }
        tree.grad_update(LEARNING_RATE);
        tree.split_tree(10);
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            double col = tree.get_col(i, j);
            image.at<double>(i,j) = col;
        }
    }

    cv::Mat output_image;
    image.convertTo(output_image, CV_8U, 255.0);
    cv::imwrite("../output/output_1.pgm", output_image);

    return 0;
}