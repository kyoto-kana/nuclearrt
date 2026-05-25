cmake_minimum_required(VERSION 3.14)
project(nuclearrt-runtime CXX)
set(FRIENDLY_NAME "NuclearRT-Runtime")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS" FORCE)
endif()

# Platform-specific compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /wd4102")
elseif(NOT EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wunused-label")
endif()

# Add cmake modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

if (EMSCRIPTEN)
    set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
    set(SDLTTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
    set(SDLTTF_PLUTOSVG OFF CACHE BOOL "" FORCE)
    set(SDLTTF_INSTALL OFF CACHE BOOL "" FORCE)

    add_link_options(
        -sMIN_WEBGL_VERSION=2
        -sMAX_WEBGL_VERSION=2
        -sALLOW_MEMORY_GROWTH=1
        -sINITIAL_MEMORY=2gb
        -sMAXIMUM_MEMORY=4096Mb
        -sSTACK_SIZE=1048576
        -sOFFSCREENCANVAS_SUPPORT=1
        -sFORCE_FILESYSTEM=1
        -sEXIT_RUNTIME=1
        -sASYNCIFY
        -lidbfs.js
    )
endif()

include(FetchContent)

function(include_dependency libName gitURL gitTag)
    FetchContent_Declare(${libName}
        GIT_REPOSITORY ${gitURL}
        GIT_TAG        ${gitTag}
        GIT_SHALLOW    FALSE
        GIT_PROGRESS   TRUE
    )

    FetchContent_MakeAvailable(${libName})
endfunction()

# =========================================================
# SDL SETUP
# Windows = SDL2
# Everything else = SDL3
# =========================================================

if(WIN32)

    # SDL2
    find_package(SDL2 QUIET)
    if (NOT SDL2_FOUND)
        message(STATUS "Getting SDL2 from Github")
        include_dependency(SDL2 https://github.com/libsdl-org/SDL.git release-2.32.8)
    else()
        message(STATUS "Using local SDL2")
    endif()

    # SDL2_image
    set(SDL2IMAGE_VENDORED ON CACHE BOOL "" FORCE)

    # Disable formats we don't need
    set(SDL2IMAGE_AVIF OFF CACHE BOOL "" FORCE)
    set(SDL2IMAGE_BMP OFF CACHE BOOL "" FORCE)
    set(SDL2IMAGE_JPG OFF CACHE BOOL "" FORCE)
    set(SDL2IMAGE_JXL OFF CACHE BOOL "" FORCE)
    set(SDL2IMAGE_PNG ON CACHE BOOL "" FORCE)
    set(SDL2IMAGE_TIF OFF CACHE BOOL "" FORCE)
    set(SDL2IMAGE_WEBP OFF CACHE BOOL "" FORCE)

    find_package(SDL2_image QUIET)

    if (NOT SDL2_image_FOUND)
        message(STATUS "Getting SDL2_image from Github")
        include_dependency(
            SDL2_image
            https://github.com/libsdl-org/SDL_image.git
            release-2.8.8
        )
    else()
        message(STATUS "Using local SDL2_image")
    endif()

    # SDL2_ttf
    find_package(SDL2_ttf QUIET)
    if (NOT SDL2_ttf_FOUND)
        message(STATUS "Getting SDL2_ttf from Github")
        include_dependency(SDL2_ttf https://github.com/libsdl-org/SDL_ttf.git release-2.24.0)
    else()
        message(STATUS "Using local SDL2_ttf")
    endif()

    add_compile_definitions(NUCLEAR_BACKEND_SDL2)

else()

    # SDL3
    find_package(SDL3 QUIET)
    if (NOT SDL3_FOUND)
        message(STATUS "Getting SDL3 from Github")
        include_dependency(SDL3 https://github.com/libsdl-org/SDL.git release-3.4.8)
    else()
        message(STATUS "Using local SDL3")
    endif()

    # SDL3_image
    set(SDLIMAGE_VENDORED ON)
    set(SDLIMAGE_AVIF OFF)
    set(SDLIMAGE_BMP OFF)
    set(SDLIMAGE_JPEG OFF)
    set(SDLIMAGE_WEBP OFF)
    set(SDLIMAGE_TIF OFF)

    find_package(SDL3_image QUIET)
    if (NOT SDL3_image_FOUND)
        message(STATUS "Getting SDL3_image from Github")
        include_dependency(SDL3_image https://github.com/libsdl-org/SDL_image.git release-3.4.4)
    else()
        message(STATUS "Using local SDL3_image")
    endif()

    # SDL3_ttf
    if(NOT EMSCRIPTEN)
        find_package(SDL3_ttf QUIET)
        if (NOT SDL3_ttf_FOUND)
            message(STATUS "Getting SDL3_ttf from Github")
            include_dependency(SDL3_ttf https://github.com/libsdl-org/SDL_ttf.git release-3.2.2)
        else()
            message(STATUS "Using local SDL3_ttf")
        endif()
    else()
        include_dependency(SDL3_ttf https://github.com/libsdl-org/SDL_ttf.git release-3.2.2)
    endif()

    add_compile_definitions(NUCLEAR_BACKEND_SDL3)

endif()

# =========================================================
# GLEW
# =========================================================

if(WIN32 OR (UNIX AND NOT APPLE AND NOT EMSCRIPTEN))
    find_package(GLEW QUIET)
    if (NOT GLEW_FOUND)
        message(STATUS "Getting GLEW from Github")

        set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
        set(glew-cmake_BUILD_STATIC ON CACHE BOOL "" FORCE)
        set(ONLY_LIBS ON CACHE BOOL "" FORCE)

        include_dependency(
            glew
            https://github.com/Perlmint/glew-cmake.git
            glew-cmake-2.2.0-1
        )
    else()
        message(STATUS "Using local GLEW")
    endif()
endif()

find_package(OpenGL REQUIRED)

# =========================================================
# PLATFORM DEFINES
# =========================================================

if(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
elseif(APPLE)
    add_compile_definitions(PLATFORM_MACOS)
elseif(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
    add_compile_definitions(PLATFORM_LINUX)
elseif(EMSCRIPTEN)
    add_compile_definitions(PLATFORM_WEB)
endif()

# Configure debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
endif()

# =========================================================
# IMGUI
# =========================================================

if(NOT EMSCRIPTEN)

    set(IMGUI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui")

    if(WIN32)
        set(IMGUI_SDL_BACKEND
            ${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp
        )
    else()
        set(IMGUI_SDL_BACKEND
            ${IMGUI_PATH}/backends/imgui_impl_sdl3.cpp
        )
    endif()

    set(IMGUI_SOURCES
        ${IMGUI_PATH}/imgui.cpp
        ${IMGUI_PATH}/imgui_demo.cpp
        ${IMGUI_PATH}/imgui_draw.cpp
        ${IMGUI_PATH}/imgui_tables.cpp
        ${IMGUI_PATH}/imgui_widgets.cpp
        ${IMGUI_SDL_BACKEND}
        ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
    )

    include_directories(
        ${IMGUI_PATH}
        ${IMGUI_PATH}/backends
    )

else()

    set(IMGUI_SOURCES "")

endif()

# =========================================================
# INCLUDE DIRECTORIES
# =========================================================

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# =========================================================
# SOURCES
# =========================================================

file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
)

file(GLOB_RECURSE HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

# =========================================================
# EXECUTABLE
# =========================================================

if(APPLE AND NOT EMSCRIPTEN)

    add_executable(
        ${PROJECT_NAME}
        MACOSX_BUNDLE
        ${SOURCES}
        ${HEADERS}
        ${IMGUI_SOURCES}
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        PRODUCT_NAME "${FRIENDLY_NAME}"
        MACOSX_BUNDLE_EXECUTABLE_NAME "${PROJECT_NAME}"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "${FRIENDLY_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.nuclearrt.${PROJECT_NAME}"
    )

else()

    add_executable(
        ${PROJECT_NAME}
        ${SOURCES}
        ${HEADERS}
        ${IMGUI_SOURCES}
    )

endif()

# =========================================================
# SDL LINKING
# =========================================================

if(WIN32)

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${sdl2_SOURCE_DIR}/include
        ${sdl2_image_SOURCE_DIR}/include
        ${sdl2_ttf_SOURCE_DIR}
        
        ${sdl2_ttf_SOURCE_DIR}/include          # Actual path inside git repo
        ${sdl2_ttf_SOURCE_DIR}/include/SDL2     # Fallback map for local targets
    )


    target_link_libraries(${PROJECT_NAME} PUBLIC
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
    )

else()

    if(NOT EMSCRIPTEN)
        target_include_directories(${PROJECT_NAME} PRIVATE ${SDL3_INCLUDE_DIRS})
        target_include_directories(${PROJECT_NAME} PRIVATE ${SDL3_image_INCLUDE_DIRS})
        target_include_directories(${PROJECT_NAME} PRIVATE ${SDL3_ttf_INCLUDE_DIRS})
    endif()

    target_link_libraries(${PROJECT_NAME} PUBLIC
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
        SDL3::Headers
    )

endif()

# =========================================================
# OUTPUT DIRECTORY
# =========================================================

if(EMSCRIPTEN)

    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "index"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output"
        SUFFIX ".html"
    )

elseif(APPLE AND NOT EMSCRIPTEN)

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

else()

    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

endif()

# =========================================================
# COPY DATA
# =========================================================

set(COPY_SOURCE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/copy/all")

if(EMSCRIPTEN)

    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/copy/web")
        list(APPEND COPY_SOURCE_DIRS
            "${CMAKE_CURRENT_SOURCE_DIR}/copy/web"
        )
    endif()

elseif(WIN32 OR APPLE OR UNIX)

    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/copy/desktop")
        list(APPEND COPY_SOURCE_DIRS
            "${CMAKE_CURRENT_SOURCE_DIR}/copy/desktop"
        )
    endif()

endif()

# Output destination
if(APPLE AND NOT EMSCRIPTEN)

    set(COPY_DEST_DIR
        "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources"
    )

elseif(MSVC)

    set(COPY_DEST_DIR
        "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
    )

else()

    set(COPY_DEST_DIR
        "${CMAKE_BINARY_DIR}/bin"
    )

endif()

set(copy_dirs_commands "")

foreach(source_dir IN LISTS COPY_SOURCE_DIRS)

    list(APPEND copy_dirs_commands
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${source_dir}"
        "${COPY_DEST_DIR}"
    )

endforeach()

if(NOT EMSCRIPTEN)

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        ${copy_dirs_commands}
        COMMENT "Copying files from copy directory to build output"
    )

else()

    foreach(source_dir IN LISTS COPY_SOURCE_DIRS)
        target_link_options(${PROJECT_NAME} PRIVATE
            "SHELL:--embed-file \"${source_dir}@/\""
        )
    endforeach()

    target_link_options(${PROJECT_NAME} PRIVATE
        "SHELL:--shell-file \"${CMAKE_CURRENT_SOURCE_DIR}/web_shell.html\""
    )

endif()

# =========================================================
# COPY SDL DLLS
# =========================================================

if(WIN32 AND NOT EMSCRIPTEN)

    message(STATUS "Copying SDL dlls to output directory")

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
            )

        endif()

    endforeach()

endif()

# =========================================================
# INSTALL
# =========================================================

if(APPLE AND NOT EMSCRIPTEN)

    install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION bin
    )

else()

    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
    )

endif()

# =========================================================
# PLATFORM-SPECIFIC LINKING
# =========================================================

if(WIN32 AND NOT EMSCRIPTEN)

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        winmm
        OpenGL::GL
        libglew_static
    )

elseif(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        X11
        OpenGL::GL
        libglew_static
        m
        pthread
    )

elseif(APPLE AND NOT EMSCRIPTEN)

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        "-framework OpenGL"
    )

endif()