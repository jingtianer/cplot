cmake_minimum_required(VERSION 3.26)
project(cplot)

set(CMAKE_C_STANDARD 11)
include_directories(svpng)
add_definitions(-DUSE_CHAR)
add_executable(cplot main.c)
