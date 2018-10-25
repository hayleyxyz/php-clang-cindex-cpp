cmake_minimum_required(VERSION 3.9)
project(php_clang_cindex)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_PREFIX_PATH "/usr/local/Cellar/llvm/6.0.1")

find_package(Clang)
find_package(LLVM REQUIRED CONFIG)

include_directories(${CLANG_INCLUDE_DIRS})

add_library(php_clang_cindex SHARED extension.cpp extension.h)

target_link_libraries(php_clang_cindex phpcpp)
target_link_libraries(php_clang_cindex libclang)