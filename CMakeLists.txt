cmake_minimum_required(VERSION 3.5)
project(mandelbrot CUDA CXX C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(mandelbrot
    src/stbi_wrapper.c
    libs/glad/glad.c
    src/Window.cpp

    libs/imgui/imgui.cpp
    libs/imgui/imgui_demo.cpp
    libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_impl_glfw.cpp
    libs/imgui/imgui_impl_opengl3.cpp
    libs/imgui/imgui_tables.cpp
    libs/imgui/imgui_widgets.cpp

    src/main.cu
)

target_include_directories(mandelbrot PUBLIC include)

target_link_libraries(mandelbrot PUBLIC ${PROJECT_SOURCE_DIR}/libs/glfw/glfw3.lib opengl32.lib User32.lib Gdi32.lib Shell32.lib)
