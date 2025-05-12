cmake_minimum_required(VERSION 3.16)

project(
    TestProject
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_BUILD_MODE "Debug")
set(CMAKE_DEBUG_POSTFIX "-g")
# set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(Velox)
add_subdirectory(App)
