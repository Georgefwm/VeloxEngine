cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(App ${APP_SOURCES})

target_include_directories(App PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(App PRIVATE Velox)
