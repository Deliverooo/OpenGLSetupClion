cmake_minimum_required(VERSION 3.19.2)
project(assimp)

add_library(assimp INTERFACE)

target_include_directories(assimp INTERFACE include ".")