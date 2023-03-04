cmake_minimum_required(VERSION 3.24)
project(untitled)

set(CMAKE_CXX_STANDARD 17)

add_executable(untitled main.c sqlite3.c)
target_link_libraries(untitled ws2_32)
