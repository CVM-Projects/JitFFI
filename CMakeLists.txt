project(JitFFI)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

aux_source_directory(source SOURCE)

add_executable(jitffi ${SOURCE} testsuite/test0.cpp)
add_executable(jitffi-test new/jitfunc.c new/testsuite/test_jitfunc.c new/jitcode_impl/x86_64.c)
