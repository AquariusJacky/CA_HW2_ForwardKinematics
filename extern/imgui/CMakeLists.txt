cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(imgui LANGUAGES CXX)

add_library(imgui STATIC
    src/imgui.cpp
    src/imgui_draw.cpp
    src/imgui_impl_glfw.cpp
    src/imgui_impl_opengl3.cpp
    src/imgui_tables.cpp
    src/imgui_widgets.cpp
)

add_dependencies(imgui glfw glad)
target_link_libraries(imgui
    PRIVATE glfw
    PRIVATE glad
)

if (NOT DEFINED COMPILER_FLAG_TEST_COMPLETE)
    message(WARNING "You should check compiler options before including this")
else()
    # Visual Studio need this for multithread compile
    if (MSVC)
        target_compile_options(imgui PRIVATE "/MP")
    endif()
    # Link time optimization
    if (COMPILER_SUPPORT_LTO)
        target_compile_options(imgui PRIVATE "-flto")
    elseif(MSVC)
        target_compile_options(imgui PRIVATE "$<$<CONFIG:Release>:/GL>")
    endif()
    # SIMD support
    if (COMPILER_SUPPORT_MARCH_NATIVE)
        target_compile_options(imgui PRIVATE "-march=native")
    elseif(COMPILER_SUPPORT_xHOST)
        target_compile_options(imgui PRIVATE "-xHost")
    elseif(COMPILER_SUPPORT_QxHOST)
        target_compile_options(imgui PRIVATE "/QxHost")
    elseif(MSVC)
        if (AVX512_RUN_RESULT EQUAL 0)
            target_compile_options(imgui PRIVATE "/arch:AVX512")
        elseif(AVX2_RUN_RESULT EQUAL 0)
            target_compile_options(imgui PRIVATE "/arch:AVX2")
        elseif(AVX_RUN_RESULT EQUAL 0)
            target_compile_options(imgui PRIVATE "/arch:AVX")
        endif()
    endif()
endif()

set_target_properties(imgui PROPERTIES CMAKE_CXX_EXTENSIONS OFF)
target_compile_features(imgui PRIVATE cxx_std_17)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD2)
target_include_directories(imgui
    PUBLIC include
    PRIVATE src
)
