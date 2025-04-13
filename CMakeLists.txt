cmake_minimum_required(VERSION 3.10)

project(HOS VERSION 0.0.1 LANGUAGES C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compiler warnings
add_compile_options(-Wall -Wextra -std=c11)

# Collect all .cpp files from src/
file(GLOB_RECURSE SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.c)

find_package(OpenSSL REQUIRED)

# Define the executable target
add_executable(HOS ${SOURCE_FILES})