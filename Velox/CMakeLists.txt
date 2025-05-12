cmake_minimum_required(VERSION 3.16)

include(FetchContent)

FetchContent_Declare(
    SDL3
    URL "https://github.com/libsdl-org/SDL/releases/download/release-3.2.12/SDL3-3.2.12.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP NEW
)
FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(
    DearImGui
    URL "https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP NEW
)
FetchContent_MakeAvailable(DearImGui)

file(GLOB_RECURSE VELOX_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*/*.cpp
)

file(GLOB_RECURSE VELOX_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Velox/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Velox/*/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Velox/*/*/*.h
)

add_library(Velox STATIC ${VELOX_SOURCES} ${VELOX_HEADERS})

target_include_directories(Velox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Velox)

# include(PrecompiledHeaders)
# target_precompile_headers(Velox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/Velox/EnginePCH.h)

target_link_libraries(Velox PRIVATE SDL3::SDL3)
