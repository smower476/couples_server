cmake_minimum_required(VERSION 3.15)
project(couples_server)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE "Release")
 
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBHTTPSERVER REQUIRED libhttpserver)

add_executable(server main.cpp)

target_link_libraries(server PRIVATE ${LIBHTTPSERVER_LIBRARIES})
