cmake_minimum_required(VERSION 3.12)
project(2d_square_adaptive)

set(CMAKE_CXX_STANDARD 11)

# Add the source files
set(SOURCES
    main.cpp
    tree.h
    vertex.h
)

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES})