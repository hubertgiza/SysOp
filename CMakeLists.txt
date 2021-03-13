cmake_minimum_required(VERSION 3.17)
project(untitled C)

set(CMAKE_C_STANDARD 99)

add_executable(untitled main.c)
add_executable(main_lab1 Lab_01/zad1/main.c)
add_library(library STATIC Lab_01/zad1/library.c)

target_link_libraries(main_lab1 library)