add_compile_definitions(NUCLEAR_BACKEND_SDL2)

# ---------------------------------------------------------
# Web options
# ---------------------------------------------------------

if(PLATFORM_WEB)
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

# ---------------------------------------------------------
# SDL2 build type
# ---------------------------------------------------------

if(PLATFORM_IOS)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
endif()

# ---------------------------------------------------------
# SDL2
# ---------------------------------------------------------

find_package(SDL2 CONFIG QUIET)

if(NOT SDL2_FOUND AND NOT TARGET SDL2::SDL2)
    message(STATUS "Getting SDL2 from GitHub")

    include_dependency(
        SDL2
        https://github.com/libsdl-org/SDL.git
        release-2.32.8
    )
else()
    message(STATUS "Using local SDL2")
endif()

# ---------------------------------------------------------
# SDL2_image
# ---------------------------------------------------------

set(SDL2IMAGE_VENDORED ON CACHE BOOL "" FORCE)

set(SDL2IMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_BMP OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_JPG OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_JXL OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_PNG ON CACHE BOOL "" FORCE)
set(SDL2IMAGE_TIF OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_WEBP OFF CACHE BOOL "" FORCE)

set(PNG_ARM_NEON "off" CACHE STRING "" FORCE)

find_package(SDL2_image CONFIG QUIET)

if(NOT SDL2_image_FOUND AND NOT TARGET SDL2_image::SDL2_image)
    message(STATUS "Getting SDL2_image from GitHub")

    include_dependency(
        SDL2_image
        https://github.com/libsdl-org/SDL_image.git
        release-2.8.8
    )
else()
    message(STATUS "Using local SDL2_image")
endif()

# ---------------------------------------------------------
# SDL2_ttf
# ---------------------------------------------------------

set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)

find_package(SDL2_ttf CONFIG QUIET)

if(NOT SDL2_ttf_FOUND AND NOT TARGET SDL2_ttf::SDL2_ttf)
    message(STATUS "Getting SDL2_ttf from GitHub")

    include_dependency(
        SDL2_ttf
        https://github.com/libsdl-org/SDL_ttf.git
        release-2.24.0
    )
else()
    message(STATUS "Using local SDL2_ttf")
endif()
