cmake_minimum_required(VERSION 3.16)

include(FetchContent)

set(DCMAKE_C_FLAGS   "-march=x86-64-v3")
set(DCMAKE_CXX_FLAGS "-march=x86-64-v3")

FetchContent_Declare(
    SDL3
    URL "https://github.com/libsdl-org/SDL/releases/download/release-3.2.14/SDL3-3.2.14.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP NEW
)

FetchContent_MakeAvailable(SDL3)

