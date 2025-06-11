cmake_minimum_required(VERSION 3.16)

include(FetchContent)

set(SDLIMAGE_VENDORED ON)
set(SDLIMAGE_AVIF OFF)
set(SDLIMAGE_INSTALL OFF)
set(SDLIMAGE_BUILD_SHARED_LIBS OFF)

FetchContent_Declare(
    SDL3_image
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL_image"
    GIT_TAG "release-3.2.4"
    GIT_SUBMODULES_RECURSE FALSE
)

FetchContent_MakeAvailable(SDL3_image)

