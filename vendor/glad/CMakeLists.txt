cmake_minimum_required(VERSION 3.14)

include(FetchContent)

FetchContent_Declare(
    glad
    URL "https://github.com/Dav1dde/glad/archive/refs/tags/v2.0.8.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP NEW
    SOURCE_SUBDIR cmake
)

FetchContent_MakeAvailable(glad)

glad_add_library(glad_using_ver SHARED API gl:core=4.6)