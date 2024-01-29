cmake_minimum_required(VERSION 3.14)
project(ccredis)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(include)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

add_executable(
    RESP_SUITE
    src/RespHandler.cpp
    test/RespHandlerEncodeTest.cpp
    test/RespHandlerDecodeTest.cpp
    )

add_executable(
    server
    src/RespHandler.cpp
    src/Server.cpp
    )
target_link_libraries(
    RESP_SUITE gtest_main
)

include(GoogleTest)
gtest_discover_tests(RESP_SUITE)

target_compile_options(RESP_SUITE PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic -Werror)