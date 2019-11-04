cmake_minimum_required(VERSION 3.0.0)
project(SamsungTask VERSION 0.1.0)

add_executable(SamsungTask main.cpp puzzle_solver.cpp)
add_executable(test test.cpp puzzle_solver.cpp)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET test PROPERTY CXX_STANDARD 17)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
