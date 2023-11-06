cmake_minimum_required(VERSION 3.14)
project(ntpclient)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(
    ntpClient
    src/NtpClient.cpp
)