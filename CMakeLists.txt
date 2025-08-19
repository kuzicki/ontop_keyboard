cmake_minimum_required(VERSION 3.30)
project(ontop_keyboard)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_MAKE_PROGRAM "ninja")
set(CMAKE_RC_COMPILER llvm-rc)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_FLAGS_RELEASE "-O3 -flto")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto")

set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/resources")
set(RESOURCE_FILE "${RESOURCE_DIR}/resource.rc")
set(RESOURCE_RES "${CMAKE_BINARY_DIR}/resource.res")

file(GLOB_RECURSE sources 
    LIST_DIRECTORIES false
    CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.h"
    "src/*.hpp"
)

add_custom_command(
    OUTPUT ${RESOURCE_RES}
    COMMAND llvm-rc /V /fo resource.res ${RESOURCE_DIR}/resource.rc
    DEPENDS ${RESOURCE_DIR}/resource.rc ${RESOURCE_DIR}/trayicon.ico
    COMMENT "Compiling resources with llvm-rc"
)

add_custom_target(resource_rc ALL DEPENDS ${RESOURCE_RES})

message(STATUS "RESOURCE_RES = ${RESOURCE_RES}")

add_executable(ontop_keyboard WIN32
    ${sources}
)
add_dependencies(ontop_keyboard resource_rc)

target_link_options(ontop_keyboard PRIVATE "${RESOURCE_RES}")

target_compile_definitions(ontop_keyboard PRIVATE UNICODE _UNICODE)

target_compile_definitions(ontop_keyboard
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
)

target_compile_options(ontop_keyboard PRIVATE
    -target x86_64-windows-msvc
    -fms-extensions
    -fms-compatibility
    -fms-compatibility-version=19.0
    -Wno-deprecated-declarations
    -Wno-microsoft-cast
    -Wno-microsoft-include
    -Wno-nonportable-include-path
    -Wno-unknown-attributes

    -Wall
    -Wextra
    -Wunused-variable
)

set_target_properties(ontop_keyboard PROPERTIES
    WIN32_EXECUTABLE ON
    LINK_FLAGS "-Wl,/SUBSYSTEM:WINDOWS -Wl,/ENTRY:WinMainCRTStartup"
)

target_include_directories(ontop_keyboard PRIVATE "${CMAKE_SOURCE_DIR}")

target_link_libraries(ontop_keyboard
    PRIVATE
        user32.lib
        gdi32.lib
        gdiplus.lib
        ws2_32.lib
)

add_custom_command(TARGET ontop_keyboard POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${RESOURCE_DIR}" "${CMAKE_BINARY_DIR}/resources"
)

add_custom_target(run
    COMMAND ontop_keyboard.exe
    DEPENDS ontop_keyboard
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)
