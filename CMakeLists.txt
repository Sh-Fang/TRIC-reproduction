cmake_minimum_required(VERSION 3.24)
project(TRIC_reproduction_master)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(TRIC_reproduction_master
        basic_struct.h
        main.cpp
        basic_struct.cpp)
