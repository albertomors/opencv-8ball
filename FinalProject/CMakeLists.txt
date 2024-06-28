cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(CVproject VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Include directories for OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(fieldDetection src/fieldDetection.cpp)

# Add executable and specify source files
add_executable(${PROJECT_NAME} src/main_ele.cpp src/fieldDetection.cpp)

link_directories(${CMAKE_CURRENT_BINARY_DIR})

# Link against OpenCV libraries and any other necessary libraries
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} fieldDetection)
