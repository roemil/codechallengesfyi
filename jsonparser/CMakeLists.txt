cmake_minimum_required(VERSION 3.14)
project(jsonparser)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(include)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
    json_SUITE
    src/json.cpp
    test/json_SUITE.cpp
)
target_link_libraries(
    json_SUITE gtest_main
)
 
include(GoogleTest)
gtest_discover_tests(json_SUITE)