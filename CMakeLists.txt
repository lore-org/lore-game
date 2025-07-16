include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)
include(CheckCXXSourceCompiles)

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
set(CLANG CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

project(lore-game
    LANGUAGES CXX
    VERSION 0.0.1
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/**.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC include)

check_cxx_compiler_flag(-fexperimental-library EXPER_LIB_SUPPORTED)
check_cxx_compiler_flag(-gcodeview CODEVIEW_SUPPORTED)
check_cxx_compiler_flag(-gcolumn-info COLUMN_INFO_SUPPORTED)
check_cxx_compiler_flag(-g CXX_DEBUG_SUPPORTED)

check_linker_flag(CXX -fuse-ld=lld USE_LLD_SUPPORTED)
check_linker_flag(CXX -g LD_DEBUG_SUPPORTED)
check_linker_flag(CXX -Wl,--pdb= PDB_SUPPORTED)

if (DEBUG)
    target_compile_options(${PROJECT_NAME} PUBLIC
        $<$<BOOL:${CXX_DEBUG_SUPPORTED}>:-g>
        $<$<BOOL:${CODEVIEW_SUPPORTED}>:-gcodeview>
        $<$<BOOL:${COLUMN_INFO_SUPPORTED}>:-gcolumn-info>
    )
    target_link_options(${PROJECT_NAME} PUBLIC
        $<$<BOOL:${USE_LLD_SUPPORTED}>:-fuse-ld=lld>
        $<$<BOOL:${LD_DEBUG_SUPPORTED}>:-g>
        $<$<BOOL:${PDB_SUPPORTED}>:-Wl,--pdb=>
    )
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

target_compile_options(${PROJECT_NAME} PUBLIC
    $<$<BOOL:${EXPER_LIB_SUPPORTED}>:-fexperimental-library>
)

check_cxx_source_compiles("
    #include <execution>
    #include <vector>

    void func(void) {
        std::vector<int> v = {};
        std::sort(std::execution::par, v.begin(), v.end());
    }
" HAS_PAR)

check_cxx_source_compiles("
    #include <execution>
    #include <vector>

    void func(void) {
        std::vector<int> v = {};
        std::sort(std::execution::par_unseq, v.begin(), v.end());
    }
" HAS_PAR_UNSEQ)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    $<$<BOOL:${HAS_PAR_UNSEQ}>:HAS_PAR>
    $<$<BOOL:${HAS_PAR_UNSEQ}>:HAS_PAR_UNSEQ>
)

set(SDL_CCACHE ON)
set(SDL_CLOCK_GETTIME ON)
if (BUILD_SHARED_LIBS)
    set(SDL_SHARED ON)
    set(SDL_STATIC OFF)
else()
    set(SDL_SHARED OFF)
    set(SDL_STATIC ON)
endif()

set(SDLIMAGE_VENDORED ON)
set(SDLIMAGE_AVIF OFF)

set(SDLTTF_VENDORED ON)

set(CPR_USE_SYSTEM_LIB_PSL ON) # otherwise CPR will try building libpsl
set(CURL_USE_LIBPSL OFF)

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

target_link_libraries(${PROJECT_NAME} PUBLIC
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
    discord-rpc
    cpr
    fmt
    glaze::glaze
)

function(copy_libraries_recurse target)
    get_target_property(libraries ${target} LINK_LIBRARIES)
    foreach(library IN LISTS libraries)
        if (
            library STREQUAL "HarfBuzz::HarfBuzz" OR
            NOT TARGET ${library}
        )
            continue()
        endif()

        copy_libraries_recurse(${library})

        get_target_property(target_type ${library} TYPE)
        if (NOT target_type STREQUAL "SHARED_LIBRARY")
            continue()
        endif()

        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${library}>
                    ${CMAKE_CURRENT_BINARY_DIR}/bin/$<TARGET_FILE_NAME:${library}>
            COMMENT "Copying $<TARGET_FILE_NAME:${library}> to build/bin"
        )
    endforeach()
endfunction()

add_custom_command(
    TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm
            -rf
            ${CMAKE_CURRENT_BINARY_DIR}/bin
            $<TARGET_FILE:${PROJECT_NAME}>
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pdb
    COMMENT "Cleaning built artifacts"
)

copy_libraries_recurse(${PROJECT_NAME})

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${PROJECT_NAME}>
            ${CMAKE_CURRENT_BINARY_DIR}/bin/$<TARGET_FILE_NAME:${PROJECT_NAME}>
    COMMENT "Copying $<TARGET_FILE_NAME:${PROJECT_NAME}> to build/bin"
)

if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pdb)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pdb
                ${CMAKE_CURRENT_BINARY_DIR}/bin/${PROJECT_NAME}.pdb
        COMMENT "Copying ${PROJECT_NAME}.pdb to build/bin"
    )
endif()

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/resources
            ${CMAKE_CURRENT_BINARY_DIR}/bin/resources
    COMMENT "Copying resources to build/bin"
)

set_property(
    DIRECTORY
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/bin"
)