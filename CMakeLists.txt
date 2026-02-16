cmake_minimum_required(VERSION 4.1)
project(data_structures C)

set(CMAKE_C_STANDARD 11)

add_library(data_structures STATIC
        linkedlist.c)
