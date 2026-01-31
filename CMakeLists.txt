
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
    find_package(pybind11 QUIET)
    if(pybind11_FOUND)
        add_subdirectory(python)
    else()
        message(WARNING "pybind11 not found, skipping Python Bindings")
    endif()
endif()