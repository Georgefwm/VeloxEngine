cmake_minimum_required(VERSION 3.16)

include(FetchContent)

set(MSDF_ATLAS_BUILD_STANDALONE OFF)
set(MSDF_ATLAS_USE_VCPKG OFF)
set(MSDF_ATLAS_USE_SKIA OFF)

#find_package(msdfgen CONFIG REQUIRED)

set(MSDFGEN_USE_SKIA    OFF)
set(MSDF_ATLAS_USE_SKIA OFF)
set(BUILD_SHARED_LIBS   ON)
set(MSDF_ATLAS_DYNAMIC_RUNTIME ON)

FetchContent_Declare(
    msdf_atlas_gen
    GIT_REPOSITORY "https://github.com/Chlumsky/msdf-atlas-gen.git"
    GIT_TAG        "v1.3"
    GIT_SUBMODULES_RECURSE FALSE
)
FetchContent_MakeAvailable(msdf_atlas_gen)

