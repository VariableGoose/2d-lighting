cmake_minimum_required(VERSION 3.31)

project(web)
set(CMAKE_BUILD_TYPE "Debug")

file(GLOB_RECURSE SOURCE CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE})

set_target_properties(${CMAKE_PROJECT_NAME}
    PROPERTIES
    C_STANDARD "99"
    C_STANDARD_REQUIRED true
    EXPORT_COMPILE_COMMANDS true
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    COMPILE_FLAGS "-Wall -Wextra"
)

if (EMSCRIPTEN)
    # This is needed in order to make intellisense work correctly.
    execute_process(COMMAND emcc --cflags OUTPUT_VARIABLE EM_CFLAGS)
    get_target_property(CFLAGS ${CMAKE_PROJECT_NAME} COMPILE_FLAGS)
    set_target_properties(${CMAKE_PROJECT_NAME}
        PROPERTIES
        COMPILE_FLAGS "${CFLAGS} ${EM_CFLAGS}"
    )
    target_link_options(${CMAKE_PROJECT_NAME}
        PRIVATE "-sMIN_WEBGL_VERSION=2"
        PRIVATE "-sMAX_WEBGL_VERSION=2"
    )
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        GLESv2 EGL
    )
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif ()

target_include_directories(web PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
