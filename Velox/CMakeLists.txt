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

if(BUILD_SHARED_LIBS)
    add_library(Velox SHARED ${VELOX_SOURCES} ${VELOX_HEADERS})
else()
    add_library(Velox STATIC ${VELOX_SOURCES} ${VELOX_HEADERS})
endif()

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
find_package(fmt          CONFIG REQUIRED)
find_package(spdlog       CONFIG REQUIRED)

target_link_libraries(Velox PUBLIC
    SDL3::SDL3
    SDL3_image::SDL3_image
    Freetype::Freetype
    imgui::imgui
    glm::glm
    glad_using_ver
    msdf-atlas-gen::msdf-atlas-gen
    tomlplusplus::tomlplusplus
    fmt::fmt
    spdlog::spdlog
)

target_link_libraries(Velox PRIVATE OpenGL::GL)

if (BUILD_SHARED_LIBS)
    message("Building velox as shared lib")
    target_compile_definitions(Velox PRIVATE VELOX_DLL VELOX_BUILD_DLL)
endif()

