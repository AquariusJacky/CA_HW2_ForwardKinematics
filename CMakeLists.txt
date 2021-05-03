cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(ForwardKinematics LANGUAGES CXX)
# Set to Release by default
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
if (NOT (CMAKE_BUILD_TYPE OR CMAKE_CONFIGURATION_TYPES))
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "RelWithDebInfo" "MinSizeRel" "Debug")
endif ()
# Detect some compiler flags
include(CheckCXXCompilerFlag)
if (NOT COMPILER_FLAG_TEST_COMPLETE)
    check_cxx_compiler_flag("-march=native" COMPILER_SUPPORT_MARCH_NATIVE)
    check_cxx_compiler_flag("-flto" COMPILER_SUPPORT_LTO)
    check_cxx_compiler_flag("-xHost" COMPILER_SUPPORT_xHOST)
    check_cxx_compiler_flag("/QxHost" COMPILER_SUPPORT_QxHOST)
    check_cxx_compiler_flag("/arch:AVX512" COMPILER_SUPPORT_ARCH_AVX512)
    set(COMPILER_FLAG_TEST_COMPLETE TRUE)
endif()
# Detect AVX2 if using Visual Studio since it doens't provided -march=native
if (MSVC)
    if (COMPILER_SUPPORT_ARCH_AVX512 AND NOT DEFINED AVX512_RUN_RESULT)
        try_run(AVX512_RUN_RESULT AVX512_COMPILE_RESULT ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cputest/avx512.cpp)
    endif()
    if (NOT DEFINED AVX2_RUN_RESULT)
        try_run(AVX2_RUN_RESULT AVX2_COMPILE_RESULT ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cputest/avx2.cpp)
    endif()
    if (NOT DEFINED AVX_RUN_RESULT)
        try_run(AVX_RUN_RESULT AVX_COMPILE_RESULT ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cputest/avx.cpp)
    endif()
endif()
# Softbody simulation part
add_executable(ForwardKinematics
    ${CMAKE_CURRENT_SOURCE_DIR}/src/acclaim/motion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/acclaim/posture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/acclaim/skeleton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/box.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/cylinder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/default_camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/free_camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/plane.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/rigidbody.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/sphere.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simulation/ball.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simulation/kinematics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util/filesystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util/helper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ForwardKinematics/main.cpp
)
# Base include files
target_include_directories(ForwardKinematics PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
# Use C++17
target_compile_features(ForwardKinematics PRIVATE cxx_std_17)
# Use std C++17 not GNU C++17
set_target_properties(ForwardKinematics PROPERTIES CMAKE_CXX_EXTENSIONS OFF)
target_compile_definitions(ForwardKinematics
    PRIVATE GLFW_INCLUDE_NONE               # Workaround for include order of glfw and glad2
    PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD2  # Help imgui to search headers
)
# Visual Studio need this for multithread compile
if (MSVC)
    target_compile_options(ForwardKinematics PRIVATE "/MP")
endif()
# Link time optimization
if (COMPILER_SUPPORT_LTO)
    target_compile_options(ForwardKinematics PRIVATE "-flto")
elseif(MSVC)
	target_compile_options(ForwardKinematics PRIVATE "$<$<CONFIG:Release>:/GL>")
	target_link_options(ForwardKinematics PRIVATE "$<$<CONFIG:Release>:/LTCG:incremental>")
endif()
# SIMD support
if (COMPILER_SUPPORT_MARCH_NATIVE)
    target_compile_options(ForwardKinematics PRIVATE "-march=native")
elseif(COMPILER_SUPPORT_xHOST)
    target_compile_options(ForwardKinematics PRIVATE "-xHost")
elseif(COMPILER_SUPPORT_QxHOST)
    target_compile_options(ForwardKinematics PRIVATE "/QxHost")
elseif(MSVC)
    if (AVX512_RUN_RESULT EQUAL 0)
        message(STATUS "Your CPU supports AVX512")
        target_compile_options(ForwardKinematics PRIVATE "/arch:AVX512")
    elseif(AVX2_RUN_RESULT EQUAL 0)
        message(STATUS "Your CPU supports AVX2")
        target_compile_options(ForwardKinematics PRIVATE "/arch:AVX2")
    elseif(AVX_RUN_RESULT EQUAL 0)
        message(STATUS "Your CPU supports AVX")
        target_compile_options(ForwardKinematics PRIVATE "/arch:AVX")
    endif()
endif()

# For exporter
find_package(Threads REQUIRED)
# Add third-party libraries
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern)
# Link those third-party libraries
target_link_libraries(ForwardKinematics
    PRIVATE Threads::Threads
    PRIVATE eigen
    PRIVATE glad
    PRIVATE glfw
    PRIVATE imgui
    PRIVATE stb
)
# Add a convienience install to ./bin
install(TARGETS ForwardKinematics RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
