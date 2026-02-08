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
    add_definitions(-DUNICODE -D_UNICODE)
    add_executable(raytracer2
            WIN32
#            main.cpp
#            ../src/LearnWin32BaseWindow.h
            main2.cpp
            ../src/orbit_camera.h
    )
#
    set_target_properties(raytracer2 PROPERTIES WIN32_EXECUTABLE TRUE
                                                LINK_FLAGS "/ENTRY:wmainCRTStartup")

    target_link_libraries(raytracer2 PRIVATE raytracer_core)
    target_link_libraries(raytracer2 PRIVATE gdi32 user32)

    if (ENABLE_TRACY)
        target_include_directories(raytracer2 PRIVATE ${tracy_SOURCE_DIR}/public)
        target_link_libraries(raytracer2 PUBLIC TracyClient)
        target_link_libraries(raytracer2 PRIVATE ws2_32 DbgHelp)
    endif()
endif()