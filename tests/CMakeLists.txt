cmake_minimum_required(VERSION 3.16)

include(FetchContent)

FetchContent_Declare(
    GTest
    URL "https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP NEW
)

FetchContent_MakeAvailable(GTest)

add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

add_executable(VeloxTests TestRunner.cpp) 
target_link_libraries(VeloxTests PRIVATE GTest::GTest Velox)

add_test(test_test VeloxTests)
