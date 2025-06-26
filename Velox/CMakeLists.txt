cmake_minimum_required(VERSION 3.16)

# Just regex all files in these dirs for simplicity.
file(GLOB_RECURSE VELOX_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*/*.cpp
)

# Same again, but for headers.
file(GLOB_RECURSE VELOX_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Velox/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Velox/*/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Velox/*/*/*.h
)

add_library(Velox STATIC ${VELOX_SOURCES} ${VELOX_HEADERS})

target_include_directories(
    Velox
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Velox
)

target_precompile_headers(Velox PUBLIC "include/Velox/PCH.h")

find_package(OpenGL              REQUIRED)
find_package(SDL3         CONFIG REQUIRED)
find_package(SDL3_image   CONFIG REQUIRED)
find_package(glm          CONFIG REQUIRED)
find_package(freetype     CONFIG REQUIRED)
find_package(imgui        CONFIG REQUIRED)
find_package(tomlplusplus CONFIG REQUIRED)

# Libs from /vendor/CMakeLists.txt
target_link_libraries(Velox PUBLIC
    SDL3::SDL3
    SDL3_image::SDL3_image
    Freetype::Freetype
    imgui::imgui
    glm::glm
    glad_using_ver
    msdf-atlas-gen::msdf-atlas-gen
    tomlplusplus::tomlplusplus
)

target_link_libraries(Velox PRIVATE OpenGL::GL)

