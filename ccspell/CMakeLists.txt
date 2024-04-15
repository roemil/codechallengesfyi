cmake_minimum_required(VERSION 3.15)

project(ccspell)

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
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(ccspell
                src/main.cpp
                #test/test.cpp
)

add_executable(ccspell_suite
               test/BloomFilterSuite.cpp
)

target_link_libraries(
    ccspell_suite gtest_main
)

include(GoogleTest)
gtest_discover_tests(ccspell_suite)

target_compile_options(ccspell_suite PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_compile_options(ccspell PRIVATE -Wall -Wextra -Wpedantic -Werror)