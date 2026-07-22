# ---------------------------------------------------------
# Validate targets
# ---------------------------------------------------------

if(NOT TARGET SDL2::SDL2)
    message(FATAL_ERROR "SDL2::SDL2 target was not created")
endif()

if(NOT TARGET SDL2_image::SDL2_image)
    message(FATAL_ERROR "SDL2_image::SDL2_image target was not created")
endif()

if(NOT TARGET SDL2_ttf::SDL2_ttf)
    message(FATAL_ERROR "SDL2_ttf::SDL2_ttf target was not created")
endif()

# ---------------------------------------------------------
# Link SDL2 libraries
# ---------------------------------------------------------

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
)

# SDL2main is primarily useful on desktop platforms where SDL
# supplies the application entry-point wrapper.
if(TARGET SDL2::SDL2main)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            SDL2::SDL2main
    )
endif()

# ---------------------------------------------------------
# Copy runtime DLLs
# ---------------------------------------------------------

if(PLATFORM_WINDOWS)
    message(STATUS "Copying SDL2 DLLs to output directory")

    foreach(sdl_target IN ITEMS
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
    )
        if(TARGET ${sdl_target})
            add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD

                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${sdl_target}>
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>

                COMMAND_EXPAND_LISTS
            )
        endif()
    endforeach()
endif()