cmake_minimum_required(VERSION 3.14)
project(ccload)

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

set(CMAKE_CXX_FLAGS "-Wall -Wextra -g")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")




add_executable(
  lb
  src/LoadBalancer.cpp
  src/main.cpp
  )
  
add_executable(
  echoServer
  src/EchoServer/EchoServer.cpp
  src/EchoServer/main.cpp
  )
    
add_library(ccloadlib
src/TcpSocket.cpp
src/LoadBalancer.cpp
src/EchoServer/EchoServer.cpp
)
target_link_libraries(
  lb PUBLIC ccloadlib
)

target_link_libraries(
  echoServer PUBLIC ccloadlib
  )

add_executable(
  lbsuite
  test/LoadBalancerTest.cpp
  )


#add_library(lbsuitelib test/LoadBalancerTest.cpp)
#target_link_libraries(
#  lbsuite PUBLIC lbsuitelib
#)


target_link_libraries(
  lbsuite PUBLIC gtest gtest_main ccloadlib
)


add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

include(GoogleTest)
gtest_discover_tests(lbsuite)

target_compile_options(lbsuite PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_compile_options(lb PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_compile_options(echoServer PRIVATE -Wall -Wextra -Wpedantic -Werror)