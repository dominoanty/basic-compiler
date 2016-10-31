cmake_minimum_required(VERSION 3.6)
project(Compiler)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES Lexer.cpp)
set(SOURCE_FILES main.cpp AST.cpp)
add_executable(Compiler ${SOURCE_FILES})