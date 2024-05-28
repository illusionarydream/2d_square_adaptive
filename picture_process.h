#ifndef PICTURE_PROCESS_H
#define PICTURE_PROCESS_H
#include <fstream>
#include <iostream>
#define gt_col(y, x) gt_col[(y) * width + (x)]

void read_grayscale_image(const char *filename, float *&gt_col, int &width, int &height) {
    std::ifstream input_imageFile(filename);
    if (input_imageFile.is_open()) {
        {
            std::string format;
            int maxVal;
            input_imageFile >> format >> width >> height >> maxVal;

            gt_col = new float[width * height];
            if (format == "P2") {
                for (int i = 0; i < height; i++) {
                    for (int j = 0; j < width; j++) {
                        int pixel;
                        input_imageFile >> pixel;
                        gt_col(i, j) = static_cast<float>(pixel) / maxVal;
                    }
                }
            } else if (format == "P5") {
                for (int i = 0; i < height; i++) {
                    for (int j = 0; j < width; j++) {
                        unsigned char pixel;
                        input_imageFile.read(reinterpret_cast<char *>(&pixel), sizeof(pixel));
                        gt_col(i, j) = static_cast<float>(pixel) / maxVal;
                    }
                }
            } else {
                std::cout << "Invalid image format. Expected P2." << std::endl;
            }
        }
    } else {
        std::cout << "Failed to open image file." << std::endl;
    }
    std::cout << "Image read from " << filename << std::endl;
}
void write_grayscale_image(const char *filename, float *gt_col, int width, int height) {
    std::ofstream output_imageFile(filename);
    if (output_imageFile.is_open()) {
        output_imageFile << "P3" << std::endl;
        output_imageFile << width << " " << height << std::endl;
        output_imageFile << "255" << std::endl;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int pixel = static_cast<int>(gt_col(i, j) * 255);
                if (pixel < 0) {
                    pixel = 0;
                } else if (pixel > 255) {
                    pixel = 255;
                }
                output_imageFile << pixel << " " << pixel << " " << pixel << std::endl;
            }
        }
    } else {
        std::cout << "Failed to open image file." << std::endl;
    }
    std::cout << "Image written to " << filename << std::endl;
}
#endif
