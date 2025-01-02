cmake_minimum_required(VERSION 3.31)

project(glad)
if (NOT EMSCRIPTEN)
    add_library(${CMAKE_PROJECT_NAME} STATIC "src/gl.c")
else ()
    add_library(${CMAKE_PROJECT_NAME} STATIC "src/gles2.c")
endif ()
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "include/")
