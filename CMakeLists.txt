cmake_minimum_required(VERSION 3.10)

project(HOS VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable compiler warnings
add_compile_options(-Wno-all -Wno-extra -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-function)

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

# Link libgit2
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGIT2 REQUIRED libgit2)

if (LIBGIT2_FOUND)
    target_include_directories(HOS PRIVATE ${LIBGIT2_INCLUDE_DIRS})
    target_link_libraries(HOS PRIVATE ${LIBGIT2_LIBRARIES})
else()
    message(FATAL_ERROR "libgit2 not found. Please install libgit2 and pkg-config.")
endif()

# Link GTK 3
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

if (GTK3_FOUND)
    target_include_directories(HOS SYSTEM PRIVATE ${GTK3_INCLUDE_DIRS})
    target_link_libraries(HOS PRIVATE ${GTK3_LIBRARIES})
    target_link_options(HOS PRIVATE ${GTK3_LDFLAGS_OTHER})
    target_compile_options(HOS PRIVATE ${GTK3_CFLAGS_OTHER})
else()
    message(FATAL_ERROR "GTK 3 not found. Please install libgtk-3-dev and pkg-config.")
endif()
