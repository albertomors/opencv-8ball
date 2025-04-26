cmake_minimum_required(VERSION 3.1)
project(CVproject VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Locate OpenCV package
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Specify include directories
include_directories(include)

# Add all .cpp files in src/ to the project
file(GLOB SOURCES "src/*.cpp")

# Set output directories for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link necessary libraries
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})