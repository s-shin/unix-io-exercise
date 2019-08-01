cmake_minimum_required(VERSION 3.0)

set(app ex002)
project(${app} CXX)

file(GLOB_RECURSE source_files *.cc)
add_executable(${app} ${source_files})
target_compile_options(${app} PUBLIC -std=c++11 -Wall -Werror -Wextra)
