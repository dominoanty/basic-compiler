cmake_minimum_required(VERSION 3.6)
project(Compiler)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -I/usr/local/include  -fPIC -fvisibility-inlines-hidden -Wall -W -Wno-unused-parameter -Wwrite-strings -Wcast-qual -Wno-missing-field-initializers -pedantic -Wno-long-long -Wno-maybe-uninitialized -Wdelete-non-virtual-dtor -Wno-comment -Werror=date-time -std=c++11 -g  -fno-exceptions -fno-rtti -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -L/usr/local/lib -lLLVMCore -lLLVMSupport -lrt -ldl -ltinfo -lpthread -lm")

set(SOURCE_FILES Lexer.cpp)
set(SOURCE_FILES AST.cpp)
set(SOURCE_FILES Parser.cpp)
set(SOURCE_FILES main.cpp )
add_executable(Compiler ${SOURCE_FILES})