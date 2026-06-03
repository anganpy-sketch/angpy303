cmake_minimum_required(VERSION 3.20)

project(ShadowWardRaylib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(raylib_USE_STATIC_LIBS ON)

find_package(raylib REQUIRED)

add_executable(ShadowWardRaylib src/raylib_main.cpp)
target_link_libraries(ShadowWardRaylib PRIVATE raylib glfw3 gdi32 opengl32 winmm)

add_custom_command(TARGET ShadowWardRaylib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:ShadowWardRaylib>/assets")

