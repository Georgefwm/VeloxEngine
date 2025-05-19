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
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Velox)

# We can worry about PCH setup when API is a bit more concrete :)
# include(PrecompiledHeaders)
# target_precompile_headers(Velox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/Velox/EnginePCH.h)

# Libs from /vendor/CMakeLists.txt
target_link_libraries(Velox PUBLIC SDL3::SDL3 ImGui)

