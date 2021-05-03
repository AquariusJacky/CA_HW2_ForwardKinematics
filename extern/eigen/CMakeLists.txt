cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(eigen LANGUAGES CXX)

add_library(eigen INTERFACE)
target_include_directories(eigen INTERFACE include)
target_compile_definitions(eigen INTERFACE EIGEN_MPL2_ONLY)
target_compile_definitions(eigen INTERFACE EIGEN_NO_DEBUG)
target_compile_definitions(eigen INTERFACE EIGEN_DONT_PARALLELIZE)
