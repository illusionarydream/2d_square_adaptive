cmake_minimum_required(VERSION 3.12)
project(2d_square_adaptive)

set(CMAKE_CXX_STANDARD 11)

# Add the source files
set(SOURCES
    main.cpp
    vertex.h
    octree.h
    varray.h
    picture_process.h
)

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES})