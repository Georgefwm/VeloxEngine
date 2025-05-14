include(FetchContent)

FetchContent_Declare(
    ImGui
    URL "https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP NEW
)

FetchContent_MakeAvailable(ImGui)

add_library(ImGui)

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
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlgpu3.cpp
)

target_link_libraries(ImGui PUBLIC SDL3::SDL3)
