
cmake_minimum_required(VERSION 3.20)
project(raytracer2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

include(FetchContent)

FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG        v0.12.2
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(tracy)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(TRACY_ENABLE)
endif()

# fast_obj C library
add_library(fastobj STATIC fast_obj.c
        )
target_include_directories(fastobj PUBLIC fast_obj)
if (MSVC)
  target_compile_definitions(fastobj PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# your app
add_executable(raytracer2
  main.cpp
  load_obj_fast.cpp
        bvh.h
        bvh.cpp
        renderer.cpp
        sh_lighting.h
        sh_lighting.cpp
)

target_include_directories(raytracer2 PRIVATE ${tracy_SOURCE_DIR}/public)
target_include_directories(raytracer2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(raytracer2 PRIVATE fastobj)
target_link_libraries(raytracer2 PUBLIC TracyClient)

if (WIN32)
  # tracy needs winsock + dbghelp
  target_link_libraries(raytracer2 PRIVATE ws2_32 DbgHelp)
  # used for rendering to WIN32 OS windows
  target_link_libraries(raytracer2 PRIVATE gdi32 user32)
endif()

