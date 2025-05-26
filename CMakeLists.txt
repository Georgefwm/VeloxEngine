cmake_minimum_required(VERSION 3.16)

project(TestProject
    VERSION 0.1.0
    LANGUAGES CXX
)

# Set unified output directories for Visual Studio (and others)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Also set per-configuration outputs (important for multi-config generators like Visual Studio)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "${CMAKE_BINARY_DIR}/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "${CMAKE_BINARY_DIR}/lib")
endforeach()

# Generate compile_commands.json which LSPs use.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Sets C++ standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Sets valid build configs.
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# Set default build to debug (generates pdb file).
set(CMAKE_BUILD_MODE "Debug")
set(CMAKE_DEBUG_POSTFIX "-g")

# Thanks Microsoft (:
add_compile_options("-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH")

add_subdirectory(vendor)

# Use CMakeLists.txt in these dirs.
add_subdirectory(Velox)
add_subdirectory(App)

if (BUILD_VELOX_TESTS)
    add_subdirectory(tests)
endif()
