cmake_minimum_required(VERSION 3.14)

include(FetchContent)

# set(DCMAKE_C_FLAGS   "-DBUILD_SHARED_LIBS=ON")
# set(DCMAKE_CXX_FLAGS "-DBUILD_SHARED_LIBS=ON")

add_compile_options("-DBUILD_SHARED_LIBS=ON")

FetchContent_Declare(
    glm
    URL "https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP NEW
)

# set(glm_DIR ${GLM_SOURCE_DIR}/lib/cmake/glm)
set(glm_DIR <installation prefix>/lib/cmake/glm)

FetchContent_MakeAvailable(glm)

