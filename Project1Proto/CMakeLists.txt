cmake_minimum_required(VERSION 3.15)
project(project1proto C)

set(CMAKE_C_STANDARD 11)

add_library(project1proto skip_list.h skip_list.h main.c)