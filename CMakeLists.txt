cmake_minimum_required(VERSION 3.12)
project(2d_square_adaptive)

set(VCPKG_ROOT "/home/xuguodong/vcpkg" CACHE STRING "Vcpkg root directory")

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the source files
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

# Find the required packages
find_package(Eigen3 REQUIRED)
find_package(OpenCV CONFIG REQUIRED)

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link the required libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Eigen3::Eigen)
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})