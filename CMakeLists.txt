cmake_minimum_required(VERSION 3.7)
project(OpenGLSetup)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp GLAD/src/glad.c
        "header files/chunk.cpp"
        "header files/chunk.h"
        "header files/shader.h"
        "header files/texture.h"
        "header files/stb_image.h"
        "header files/stb_image.cpp"
        "header files/camera.h"
        "header files/block.h"
        "header files/mesh.h"
        "header files/vao.h")

set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${dir}/build)

include_directories(include)

#Include library files
include_directories(GLAD/include)
include_directories(glm/include)

#Add deps
add_subdirectory(dependencies)

add_executable(OpenGLSetup ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(OpenGLSetup glfw)
