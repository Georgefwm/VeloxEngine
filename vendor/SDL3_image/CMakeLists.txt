cmake_minimum_required(VERSION 3.16)

include(FetchContent)

FetchContent_Declare(
    SDL3_image
    URL "https://github.com/libsdl-org/SDL_image/releases/download/release-3.2.4/SDL3_image-3.2.4.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP NEW
)

FetchContent_MakeAvailable(SDL3_image)

