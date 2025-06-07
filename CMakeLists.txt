cmake_minimum_required(VERSION 3.19.2)
project(OpenGLTing)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp
    "header files/stb_image.h"
    "header files/stb_image.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "OpenGLTing/build")

#Add deps
add_subdirectory("dependencies/glfw-3.4")
add_subdirectory("dependencies/glm")
add_subdirectory("dependencies/GLAD")
add_subdirectory("dependencies/imgui")
add_subdirectory("include")

add_executable(OpenGLTing ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(OpenGLTing PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/header files/")

target_link_libraries(OpenGLTing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp/assimp-vc143-mt.lib glm glad glfw imgui)

#Include library files
include_directories("include")

