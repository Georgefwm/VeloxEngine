cmake_minimum_required(VERSION 3.16)

find_package(Vulkan REQUIRED)

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
    ${Vulkan_INCLUDE_DIRS}
)

target_precompile_headers(Velox PUBLIC "include/Velox/PCH.h")

# Libs from /vendor/CMakeLists.txt
target_link_libraries(Velox PUBLIC SDL3::SDL3 SDL3_image::SDL3_image ImGui glm::glm)

target_link_libraries(Velox PRIVATE Vulkan::Vulkan)

