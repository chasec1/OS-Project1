cmake_minimum_required(VERSION 3.15)
project(project1proto C)

set(CMAKE_C_STANDARD 11)

add_executable(project1proto skip_list.h main.c)
add_executable(Driver1 ../proj1tests/Driver1.c)
add_executable(Driver2 ../proj1tests/Driver2.c)