cmake_minimum_required(VERSION 3.15)

project(ccgrep)

set(CMAKE_CXX_STANDARD 23)

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

set(CMAKE_CXX_FLAGS "-g")

add_executable(ccgrep
                src/CcGrep.cpp
                #test/test.cpp
)

#target_link_libraries(
#    ccgrep gtest_main
#)

#include(GoogleTest)
#gtest_discover_tests(ccgrep)


target_compile_options(ccgrep PRIVATE -Wall -Wextra -Wpedantic -Werror)