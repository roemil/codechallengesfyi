cmake_minimum_required(VERSION 3.15)

project(ccirc)

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
set(CMAKE_CXX_FLAGS "-g")

include(FetchContent)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v5.0.0
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_library(tui_lib STATIC
            src/Tui.cpp
)


add_executable(tui
               src/tuimain.cpp
               src/Tui.cpp
)

add_executable(ccirc
               src/ircmain.cpp
)

add_library(
  ccirc_lib STATIC
  src/IrcClient.cpp
)



target_link_libraries(
    ccirc ccirc_lib tui_lib
)



add_executable(ccirc_suite
               test/IrcProtocolSuite.cpp
               test/IrcParserSuite.cpp
)

target_link_libraries(
    ccirc_suite gtest_main gmock_main
)

target_link_libraries(
    tui
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component # Not needed for this example.
)

target_link_libraries(
    tui_lib
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component # Not needed for this example.
)

include(GoogleTest)
gtest_discover_tests(ccirc_suite)


target_compile_options(ccirc PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_compile_options(ccirc_suite PRIVATE -Wall -Wextra -Wpedantic -Werror)