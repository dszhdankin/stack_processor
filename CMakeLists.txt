cmake_minimum_required(VERSION 3.16)
project(stack_processor)

set(CMAKE_CXX_STANDARD 14)

add_compile_options(-Wall -Wextra)

add_subdirectory(src)
add_subdirectory(tests)