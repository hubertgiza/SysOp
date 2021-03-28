cmake_minimum_required(VERSION 3.17)
project(SysOp C)

set(CMAKE_C_STANDARD 99)

add_executable(SysOp main.c)
add_executable(main_lab1 Lab_01/zad_1/main.c)

add_library(library STATIC Lab_01/zad_1/library.c)
add_library(library_03 STATIC Lab_03/zad_2/library.c)

add_executable(l2_z1 Lab_02/zad_1/zad_1.c)
add_executable(l2_z1b Lab_02/zad_1/zad_1_system.c)
add_executable(l2_z2 Lab_02/zad_2/zad_2.c)
add_executable(l2_z2b Lab_02/zad_2/zad_2_system.c)
add_executable(l2_z3 Lab_02/zad_3/zad_3.c)
add_executable(l2_z3b Lab_02/zad_3/zad_3_system.c)
add_executable(l2_z4 Lab_02/zad_4/zad_4.c)
add_executable(l2_z4b Lab_02/zad_4/zad_4_system.c)
add_executable(l2_z5 Lab_02/zad_5/zad_5.c)
add_executable(l2_z5b Lab_02/zad_5/zad_5_system.c)
add_executable(l3_z1 Lab_03/zad_1/zad_1.c)
add_executable(l3_z2 Lab_03/zad_2/zad_2.c)
add_executable(l3_z3 Lab_03/zad_3/zad_3.c)

target_link_libraries(l3_z2 library_03)
target_link_libraries(main_lab1 library)
