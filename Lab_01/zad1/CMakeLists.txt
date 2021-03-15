cmake_minimum_required(VERSION 3.17)
project(untitled C)

set(CMAKE_C_STANDARD 99)
add_executable(main.c)
add_library(library SHARED library.c)

target_link_libraries(main_lab1 library)