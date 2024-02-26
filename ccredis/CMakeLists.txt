cmake_minimum_required(VERSION 3.14)
project(ccredis)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 23)

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
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(
    RESP_SUITE
    src/RespDecoder.cpp
    src/RespEncoder.cpp
    src/CommandParsePayload.cpp
    src/CommandHandler.cpp
    src/Resp.cpp
    test/RespHandlerEncodeTest.cpp
    test/RespHandlerDecodeTest.cpp
    test/RespCommandConverterTest.cpp
    test/DbTest.cpp
    )

add_executable(
    server
    src/RespDecoder.cpp
    src/Resp.cpp
    src/RespEncoder.cpp
    src/CommandParsePayload.cpp
    src/CommandHandler.cpp
    src/Server.cpp
    )
target_link_libraries(
    RESP_SUITE gtest_main
)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

include(GoogleTest)
gtest_discover_tests(RESP_SUITE)

target_compile_options(RESP_SUITE PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic -Werror)