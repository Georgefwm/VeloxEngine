cmake_minimum_required(VERSION 3.16)

include(FetchContent)

FetchContent_Declare(
    ImGui
    URL "https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP NEW
)

FetchContent_MakeAvailable(ImGui)

find_package(Vulkan REQUIRED)

add_library(ImGui STATIC)

target_include_directories(ImGui
    PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# GM: Extra ImGui features need to be manually added here.
target_sources(
    ImGui
    PUBLIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

target_link_libraries(ImGui PRIVATE SDL3::SDL3 Vulkan::Vulkan)
