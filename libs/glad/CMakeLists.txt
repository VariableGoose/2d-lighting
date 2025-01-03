cmake_minimum_required(VERSION 3.31)

add_library(glad STATIC)

target_include_directories(glad PUBLIC "include/")

if (NOT EMSCRIPTEN)
    target_sources(glad PRIVATE "src/gl.c")
else ()
    target_sources(glad PRIVATE "src/gles2.c")
endif ()
