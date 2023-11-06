cmake_minimum_required(VERSION 3.14)
project(ccwc)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(
    ccwc
    src/main.cpp
    src/CommandVisitor.cpp
    src/Counters.cpp
)
target_link_libraries(
    ccwc
)