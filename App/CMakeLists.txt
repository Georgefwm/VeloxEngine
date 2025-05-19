cmake_minimum_required(VERSION 3.16)

# Just regex all files in these dirs for simplicity.
file(GLOB_RECURSE APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*/*.cpp
)

# Same again, but for headers.
file(GLOB_RECURSE APP_HEADERS
    ${PROJECT_SOURCE_DIR}/include/App/*.h
    ${PROJECT_SOURCE_DIR}/include/App/*/*.h
    ${PROJECT_SOURCE_DIR}/include/App/*/*/*.h
)

add_executable(App ${APP_SOURCES} ${APP_HEADERS})

target_include_directories(App PRIVATE ${PROJECT_SOURCE_DIR}/include/App)

target_link_libraries(App PRIVATE Velox ImGui)
