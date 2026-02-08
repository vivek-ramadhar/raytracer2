
cmake_minimum_required(VERSION 3.20)
project(raytracer2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)
option(BUILD_APPS "Build standalone applications" ON)
option(ENABLE_TRACY "Enable Tracy profiler" OFF)

include(FetchContent)

if (ENABLE_TRACY)
    FetchContent_Declare(
            tracy
            GIT_REPOSITORY https://github.com/wolfpld/tracy.git
            GIT_TAG        v0.12.2
            GIT_SHALLOW    TRUE
            GIT_PROGRESS   TRUE
    )
    FetchContent_MakeAvailable(tracy)
    add_compile_definitions(TRACY_ENABLE)
endif()

add_subdirectory(src)

if (BUILD_APPS)
    add_subdirectory(apps)
endif()

if (BUILD_PYTHON_BINDINGS)
#    find_package(pybind11 QUIET)
    FetchContent_Declare(
            pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11.git
            GIT_TAG v2.11.1
    )
    FetchContent_MakeAvailable(pybind11)

    pybind11_add_module(sh_engine python/bindings.cpp)
    target_link_libraries(sh_engine PRIVATE raytracer_core)

    set_target_properties(raytracer_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(fastobj PROPERTIES POSITION_INDEPENDENT_CODE ON)

    if(WIN32)
        target_link_libraries(sh_engine PRIVATE gdi32 user32)
    endif()

endif()