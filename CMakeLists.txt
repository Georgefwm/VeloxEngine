cmake_minimum_required(VERSION 3.16)

project(TestProject
    VERSION 0.1.0
    LANGUAGES CXX C
)

option(VELOX_BUILD_TESTS "Generate test executable" OFF)

# Generate compile_commands.json which LSPs use.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Sets C++ standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(_ITERATOR_DEBUG_LEVEL=2 _HAS_ITERATOR_DEBUGGING=1)
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY_DEBUG "MultiThreadedDebugDLL")
set(CMAKE_MSVC_RUNTIME_LIBRARY_RELEASE "MultiThreadedDLL")

#message("Compiler: ${CMAKE_CXX_COMPILER_ID}")
#message("BuildDir: ${CMAKE_BINARY_DIR}")
#message("BuildType: ${CMAKE_BUILD_TYPE}")

# Set output dirs.
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE_UPPER)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${BUILD_TYPE_UPPER} "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${BUILD_TYPE_UPPER} "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${BUILD_TYPE_UPPER} "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}")

# Thanks Microsoft (:
add_compile_options("-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH")

add_subdirectory(vendor)

# Use CMakeLists.txt in these dirs.
add_subdirectory(Velox)
add_subdirectory(App)

if (VELOX_BUILD_TESTS)
    add_subdirectory(tests)
endif()
