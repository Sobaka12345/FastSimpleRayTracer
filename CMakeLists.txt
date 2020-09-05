cmake_minimum_required(VERSION 3.5)

project(RayTracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(glm)

add_executable(RayTracer main.cpp
                         figures.hpp
                         lights.hpp
                         Renderer.hpp
                         Renderer.cpp)

target_link_libraries(RayTracer pthread)
target_link_libraries(RayTracer sfml-graphics sfml-system sfml-window)
