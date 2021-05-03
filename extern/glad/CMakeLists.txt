cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(glad LANGUAGES C)

add_library(glad INTERFACE)
target_include_directories(glad INTERFACE include)
if (MSVC)
    target_compile_options(glad INTERFACE "/wd4551")
endif()
