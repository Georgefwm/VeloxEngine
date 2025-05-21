cmake_minimum_required(VERSION 3.16)

project(TestProject
    VERSION 0.1.0
    LANGUAGES CXX
)

# Specify where to put SDL-g.dll (needs to be in same dir as App.exe).
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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

