cmake_minimum_required(VERSION 3.24)
project(HashMap)

set(CMAKE_CXX_STANDARD 17)

add_executable(HashMap hash_map.h tester.cpp HashMap.h)
