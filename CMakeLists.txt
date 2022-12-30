cmake_minimum_required(VERSION 3.23)
project(UTool)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O2")
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(UTool main.cpp editor.h func.h)
