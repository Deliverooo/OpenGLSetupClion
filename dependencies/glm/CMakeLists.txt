cmake_minimum_required(VERSION 3.19.2)
project(glm)

add_library(glm INTERFACE)

target_include_directories(glm INTERFACE include ".")