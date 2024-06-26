#ifndef PICTURE_PROCESS_H
#define PICTURE_PROCESS_H
#include <fstream>
#include <iostream>

// grayscale image
class PictureProcess {
   public:
    // data structure to store the image
    int width;
    int height;
    double **map;

   public:
    // functions
    PictureProcess() {
        width = 0;
        height = 0;
        map = nullptr;
    }
    void read_image(const char *filename) {
        std::ifstream input_imageFile(filename);
        if (input_imageFile.is_open()) {
            {
                std::string format;
                int maxVal;
                input_imageFile >> format >> width >> height >> maxVal;
                if (format == "P2") {
                    map = new double *[height];
                    for (int i = 0; i < height; i++) {
                        map[i] = new double[width];
                        for (int j = 0; j < width; j++) {
                            int pixel;
                            input_imageFile >> pixel;
                            map[i][j] = static_cast<double>(pixel) / maxVal;
                        }
                    }
                } else if (format == "P5") {
                    map = new double *[height];
                    for (int i = 0; i < height; i++) {
                        map[i] = new double[width];
                        for (int j = 0; j < width; j++) {
                            unsigned char pixel;
                            input_imageFile.read(reinterpret_cast<char *>(&pixel), sizeof(pixel));
                            map[i][j] = static_cast<double>(pixel) / maxVal;
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
    void write_image(const char *filename) {
        std::ofstream output_imageFile(filename);
        if (output_imageFile.is_open()) {
            output_imageFile << "P3" << std::endl;
            output_imageFile << width << " " << height << std::endl;
            output_imageFile << "255" << std::endl;
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    int pixel = static_cast<int>(map[i][j] * 255);
                    output_imageFile << pixel << " " << pixel << " " << pixel << std::endl;
                }
            }
        } else {
            std::cout << "Failed to open image file." << std::endl;
        }
        std::cout << "Image written to " << filename << std::endl;
    }
};
#endif
