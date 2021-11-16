project(JitFFI)
cmake_minimum_required(VERSION 3.0)

include_directories(include)

aux_source_directory(source SOURCE)

add_executable(jitffi ${SOURCE} testsuite/test1.cpp)
