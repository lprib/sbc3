cmake_minimum_required(VERSION 3.25)
project(vm LANGUAGES CXX)

set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_CXX_STANDARD 23)

add_subdirectory(engine)
add_subdirectory(pc_port)
add_subdirectory(tests)