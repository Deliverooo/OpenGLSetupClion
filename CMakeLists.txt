cmake_minimum_required(VERSION 3.7)
project(OpenGLTing)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp GLAD/src/glad.c
        "header files/shader.h"
        "header files/stb_image.h"
        "header files/stb_image.cpp"
        "header files/camera.h"
        "header files/mesh.h"
        "header files/model.h"
        "header files/transform.h"
        "header files/entity.h")

set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${dir}/build)

#Include library files
include_directories(GLAD/include)
include_directories(glm/include)
include_directories(include)

#Add deps
add_subdirectory(dependencies)

add_executable(OpenGL_Ting ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(OpenGL_Ting glfw)

target_link_libraries(OpenGL_Ting ${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp/assimp-vc143-mt.lib)
