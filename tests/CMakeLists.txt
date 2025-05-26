cmake_minimum_required(VERSION 3.16)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL "https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP NEW
)

# For Windows: Prevent overriding the parent project's compiler/linker settings. (from start page)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(VeloxTests TestRunner.cpp) 
target_link_libraries(VeloxTests PUBLIC GTest::gtest_main Velox)

include(GoogleTest)
gtest_discover_tests(VeloxTests)

