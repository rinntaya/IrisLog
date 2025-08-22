cmake_minimum_required(VERSION 3.28)
project(Test)

set(CMAKE_CXX_STANDARD 17)

add_executable(Test main.cpp)

target_link_libraries(Test PRIVATE IrisLog)

