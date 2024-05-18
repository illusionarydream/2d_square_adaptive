#include "tree.h"
#define LEARNING_RATE 0.001
#define ITERATIONS 1000

int width;
int height;
double map[100][100];

void initial_vertex() {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            Vertex[i][j].col = 0;
            Vertex[i][j].grad_col = 0;
        }
    }
}

int main() {
    std::ifstream input_imageFile("../image_input.pgm");
    if (input_imageFile.is_open()) {
        {
            std::string format;
            int maxVal;
            input_imageFile >> format >> width >> height >> maxVal;
            if (format == "P2") {
                for (int x = 0; x < height; x++) {
                    for (int y = 0; y < width; y++) {
                        int pixel;
                        input_imageFile >> pixel;
                        map[x][y] = static_cast<double>(pixel) / maxVal;
                    }
                }
            } else {
                std::cout << "Invalid image format. Expected P2." << std::endl;
            }
        }
    } else {
        std::cout << "Failed to open image file." << std::endl;
    }

    printf("width: %d, height: %d\n", width, height);

    Tree tree(0, height, 0, width);
    initial_vertex();

    for (int i = 0; i < ITERATIONS; i++) {
        for (int x = 0; x < height; x++) {
            for (int y = 0; y < width; y++) {
                tree.grad_backwards(x, y, map[x][y]);
            }
        }
        tree.grad_update(LEARNING_RATE);
        tree.split_tree(10);
    }

    std::ofstream output_imageFile("../image.ppm");
    if (output_imageFile.is_open()) {
        output_imageFile << "P3" << std::endl;
        output_imageFile << width << " " << height << std::endl;
        output_imageFile << "255" << std::endl;
        for (int x = 0; x < height; x++) {
            for (int y = 0; y < width; y++) {
                int color = static_cast<int>(tree.get_col(x, y) * 255);
                output_imageFile << color << " " << color << " " << color << " ";
            }
            output_imageFile << std::endl;
        }
        output_imageFile.close();
    } else {
        std::cout << "Failed to open image file." << std::endl;
    }
    return 0;
}