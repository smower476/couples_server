cmake_minimum_required(VERSION 3.15)
project(couples_server)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE "Release")
 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBHTTPSERVER REQUIRED libhttpserver)
pkg_check_modules(LIBJWT REQUIRED libjwt)
pkg_check_modules(LIBPQXX REQUIRED libpqxx)
pkg_check_modules(SODIUM REQUIRED libsodium)

file(GLOB SOURCES "src/*.cpp")

add_executable(server main.cpp ${SOURCES} )

target_link_libraries(server PRIVATE ${LIBHTTPSERVER_LIBRARIES} ${LIBJWT_LIBRARIES} ${LIBPQXX_LIBRARIES} ${SODIUM_LIBRARIES})
