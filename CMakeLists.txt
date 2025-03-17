cmake_minimum_required(VERSION 3.15)
project(couples_server)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE "Release")
 
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBHTTPSERVER REQUIRED libhttpserver)
pkg_check_modules(LIBJWT REQUIRED libjwt)
pkg_check_modules(LIBPQXX REQUIRED libpqxx)
pkg_check_modules(SODIUM REQUIRED libsodium)

file(GLOB SOURCES "src/*.cpp")

add_executable(server main.cpp ${SOURCES} )

target_link_libraries(server PRIVATE ${LIBHTTPSERVER_LIBRARIES} ${LIBJWT_LIBRARIES} ${LIBPQXX_LIBRARIES} ${SODIUM_LIBRARIES})
