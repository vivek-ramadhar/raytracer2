add_library(raytracer_core STATIC
        renderer.cpp
        renderer.h
        bvh.cpp
        bvh.h
        load_obj_fast.cpp
        load_obj_fast.h
        sh_lighting.cpp
        sh_lighting.h
)

add_library(fastobj STATIC
        fast_obj.c
        fast_obj.h
)

target_include_directories(fastobj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if (MSVC)
    target_compile_directories(fastobj PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

target_link_libraries(raytracer_core PUBLIC fastobj)

if (ENABLE_TRACY)
    target_include_directories(raytracer_core PRIVATE ${tracy_SOURCE_DIR}/public)
    target_link_libraries(raytracer_core PUBLIC TracyClient)
    if (WIN32)
        target_link_libraries(raytracer_core PRIVATE ws2_32 DbgHelp)
    endif()
endif()

target_include_directories(raytracer_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})