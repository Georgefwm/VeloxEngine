cmake_minimum_required(VERSION 3.16)

include(FetchContent)

set(SDL3IMAGE_VENDORED OFF)
set(SDL3IMAGE_AVIF OFF)

set(SDL3IMAGE_INSTALL OFF)
set(SDL3IMAGE_BUILD_SHARED_LIBS OFF)
set(BUILD_SHARED_LIBS OFF)

FetchContent_Declare(
    SDL3_image
    URL "https://github.com/libsdl-org/SDL_image/releases/download/release-3.2.4/SDL3_image-3.2.4.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP NEW
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(SDL3_image)

