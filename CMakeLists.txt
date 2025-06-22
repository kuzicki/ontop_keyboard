cmake_minimum_required(VERSION 3.30)

project(ontop_keyboard)


set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_MAKE_PROGRAM "ninja")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


file(GLOB_RECURSE sources 
    LIST_DIRECTORIES false
    CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.h"
    "src/*.hpp"
)

add_executable(ontop_keyboard WIN32
    ${sources}
)

add_definitions(-DUNICODE -D_UNICODE)
target_compile_definitions(ontop_keyboard PRIVATE UNICODE _UNICODE)

target_link_libraries(ontop_keyboard
    PRIVATE
        user32.lib
        gdi32.lib
        gdiplus.lib
)

set_target_properties(ontop_keyboard PROPERTIES
    WIN32_EXECUTABLE ON
    LINK_FLAGS "-Wl,/SUBSYSTEM:WINDOWS -Wl,/ENTRY:WinMainCRTStartup"
)

target_compile_options(ontop_keyboard PRIVATE
    -Wno-deprecated-declarations
    -Wno-microsoft-cast
    -Wno-microsoft-include
    -Wno-nonportable-include-path
    -Wno-unknown-attributes
)


set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/resources")

set(RESOURCE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/resources")

add_custom_command(TARGET ontop_keyboard POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${RESOURCE_DIR}" "${RESOURCE_OUTPUT_DIR}"
)
