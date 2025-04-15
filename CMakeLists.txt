cmake_minimum_required(VERSION 3.10)

project(HOS VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable compiler warnings
add_compile_options(-Wall -Wextra)

# Collect all .c files from src/
file(GLOB_RECURSE SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.c)

# Define the executable target
add_executable(HOS ${SOURCE_FILES})

target_compile_definitions(HOS PRIVATE _DEFAULT_SOURCE _POSIX_C_SOURCE=200809L)

# Link OpenSSL
find_package(OpenSSL REQUIRED)
target_link_libraries(HOS PRIVATE OpenSSL::Crypto)

# Link Readline
find_library(READLINE_LIBRARY readline)
find_path(READLINE_INCLUDE_DIR readline/readline.h)

if (READLINE_LIBRARY AND READLINE_INCLUDE_DIR)
    target_include_directories(HOS PRIVATE ${READLINE_INCLUDE_DIR})
    target_link_libraries(HOS PRIVATE ${READLINE_LIBRARY})
else()
    message(FATAL_ERROR "Readline library not found.")
endif()
