cmake_minimum_required(VERSION 3.22)
project(TODESHAND)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS_INIT "-Werror -Wall -Wextra -Wpedantic -O3")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_SUPPRESS_REGENERATION true)

include_directories(include include/imgui)
link_directories(lib)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
find_package(SDL3_ttf REQUIRED CONFIG REQUIRED COMPONENTS SDL3_ttf-shared)

add_executable(program
    src/imgui/imgui.cpp
    src/imgui/imgui_demo.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_impl_sdl3.cpp
    src/imgui/imgui_impl_sdlrenderer3.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    src/game.cpp
    src/main.cpp
    src/sound_manager.cpp
    src/text_manager.cpp
)

target_link_libraries(program PRIVATE 
    SDL3_ttf::SDL3_ttf 
    SDL3::SDL3 
)
