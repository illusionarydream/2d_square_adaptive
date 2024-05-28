cmake_minimum_required(VERSION 3.12)
project(2d_square_adaptive CXX CUDA)

enable_language(CUDA)

include_directories(/opt/cuda/targets/x86_64-linux/include)
link_directories(/opt/cuda/targets/x86_64-linux/lib)

set(CMAKE_CXX_STANDARD 11)

# Add the source files
set(SOURCES
    main.cpp
    grad_in_gpu.cu
)

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES})