add_executable(render_to_ppm
        render_to_file.cpp
)

target_link_libraries(render_to_ppm PRIVATE raytracer_core)

if(ENABLE_TRACY)
    target_include_directories(render_to_ppm PRIVATE ${tracy_SOURCE_DIR}/public)
    target_link_libraries(render_to_ppm PUBLIC TracyClient)
    if (WIN32)
        target_link_libraries(render_to_ppm PRIVATE ws2_32 DbgHelp)
    endif()
endif()

if (WIN32)
    add_executable(raytracer2
            main.cpp
    )

    target_link_libraries(raytracer2 PRIVATE raytracer_core)
    target_link_libraries(raytracer2 PRIVATE gdi32 user32)

    if (ENABLE_TRACY)
        target_include_directories(raytracer2 PRIVATE ${tracy_SOURCE_DIR}/public)
        target_link_libraries(raytracer2 PUBLIC TracyClient)
        target_link_libraries(raytracer2 PRIVATE ws2_32 DbgHelp)
    endif()
endif()