cmake_minimum_required(VERSION 3.5)
project(mandelbrot CUDA C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(mandelbrot
    src/stbi_wrapper.c
    src/main.cu
    src/Window.cpp
    libs/glad/glad.c

    libs/imgui/imgui.cpp
    libs/imgui/imgui_demo.cpp
    libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_impl_glfw.cpp
    libs/imgui/imgui_impl_opengl3.cpp
    libs/imgui/imgui_tables.cpp
    libs/imgui/imgui_widgets.cpp
)

target_include_directories(mandelbrot PUBLIC include)

target_link_libraries(mandelbrot PUBLIC D:/Programas/CUDA/mandelbrot/libs/glfw/glfw3.lib opengl32.lib User32.lib Gdi32.lib Shell32.lib)
