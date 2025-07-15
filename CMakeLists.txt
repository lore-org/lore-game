cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(COMPILER_LAUNCHER sccache)
if (NOT COMPILER_LAUNCHER)
    find_program(COMPILER_LAUNCHER ccache)
endif()

if (COMPILER_LAUNCHER)
    set(CMAKE_C_COMPILER_LAUNCHER ${COMPILER_LAUNCHER} CACHE INTERNAL "")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${COMPILER_LAUNCHER} CACHE INTERNAL "")
endif()

if (NOT DEFINED BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ON)
endif()

find_program(PYTHON py python3 python REQUIRED)
execute_process(
    COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/generate-config.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set(DEBUG CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
set(GCC CMAKE_C_COMPILER_ID STREQUAL "GNU")

project(lore-game
    LANGUAGES CXX
    VERSION 0.0.1
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/**.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC include)

set(SDL_CCACHE ON)
set(SDL_CLOCK_GETTIME ON)
set(SDL_STATIC ON)
set(SDL_SHARED ON)

set(SDLIMAGE_VENDORED ON)
set(SDLIMAGE_AVIF OFF)

set(SDLTTF_VENDORED ON)

set(CPR_USE_SYSTEM_LIB_PSL ON)

find_package(usb)
if (NOT usb_FOUND)
    add_subdirectory(submodules/usb)
endif()

find_package(brotli)
if (NOT brotli_FOUND)
    add_subdirectory(submodules/brotli)
endif()

add_subdirectory(submodules/SDL3)
add_subdirectory(submodules/SDL3_image)
add_subdirectory(submodules/SDL3_ttf)
add_subdirectory(submodules/discord-rpc)
add_subdirectory(submodules/cpr)

find_package(fmt)
if (NOT fmt_FOUND)
    add_subdirectory(submodules/fmt)
endif()

find_package(glaze)
if (NOT glaze_FOUND)
    add_subdirectory(submodules/glaze)
endif()

if (NOT DEBUG)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
    discord-rpc
    cpr
    fmt
    glaze::glaze
)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/install-bin.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    USES_TERMINAL
    COMMENT "Copy needed files to build/bin"
)