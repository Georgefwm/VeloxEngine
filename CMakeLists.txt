cmake_minimum_required(VERSION 3.16)

project(Velox
    VERSION 0.1.0
    LANGUAGES CXX)

include(FetchContent)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_BUILD_MODE "Debug")
set(CMAKE_DEBUG_POSTFIX "-g")
# set_property(GLOBAL PROPERTY USE_FOLDERS ON)


FetchContent_Declare(
    SDL3 
    URL "https://github.com/libsdl-org/SDL/releases/download/release-3.2.12/SDL3-3.2.12.zip"
)

FetchContent_MakeAvailable(SDL3)

add_subdirectory(etern/SDL EXCLUDE_FROM_ALL)


add_executable(App WIN32 app.cpp)

target_link_libraries(hello PRIVATE SDL3::SDL3)
