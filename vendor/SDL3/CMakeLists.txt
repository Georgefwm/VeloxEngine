cmake_minimum_required(VERSION 3.16)

include(FetchContent)

FetchContent_Declare(
    SDL3
    URL "https://github.com/libsdl-org/SDL/releases/download/release-3.2.12/SDL3-3.2.12.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP NEW
)

FetchContent_MakeAvailable(SDL3)
