cmake_minimum_required(VERSION 3.15)
project(project01 C)

set(CMAKE_C_STANDARD 11)

add_library(project01 skip_list.c skip_list.h)