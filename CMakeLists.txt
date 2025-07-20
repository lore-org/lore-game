include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)
include(CheckCXXSourceCompiles)

cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_ANDROID_ARCH_ABI arm64-v8a CACHE INTERNAL "What architecture to compile Android app for")
set(ANDROID_ABI ${CMAKE_ANDROID_ARCH_ABI} CACHE INTERNAL "") # SDL relies on ANDROID_ABI instead of CMAKE_ANDROID_ARCH_ABI
set(CMAKE_ANDROID_API 29 CACHE INTERNAL "Target Android API version. Default is Android 10.")
set(CMAKE_ANDROID_API_MIN 24 CACHE INTERNAL "Minimum Android API version. Default is Android 7.0.")

if (NOT DEFINED ANDROID_NDK AND NOT DEFINED ENV{ANDROID_NDK})
    message(FATAL_ERROR "ANDROID_NDK must be defined as a cmake variable or environment variable.")
endif()
if (NOT DEFINED ANDROID_NDK)
    set(ANDROID_NDK $ENV{ANDROID_NDK} CACHE INTERNAL "") # libwebp relies on a cmake ANDROID_NDK instead of an env ANDROID_NDK
endif()

find_program(COMPILER_LAUNCHER sccache)
if (NOT COMPILER_LAUNCHER)
    find_program(COMPILER_LAUNCHER ccache)
endif()

if (COMPILER_LAUNCHER)
    set(CMAKE_C_COMPILER_LAUNCHER ${COMPILER_LAUNCHER} CACHE INTERNAL "")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${COMPILER_LAUNCHER} CACHE INTERNAL "")
endif()

set(BUILD_SHARED_LIBS ON CACHE INTERNAL "Build .dlls, .so, and .dylibs." )

find_program(PYTHON py python3 python REQUIRED)
execute_process(
    COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/generate-config.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set(DEBUG CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")

project(lore-game
    LANGUAGES C CXX
    VERSION 0.0.1
)

if (ANDROID)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/submodules/AndroidPackaging/CMake")

    include(AndroidPackaging-Prerequisites)
    include(AndroidPackaging)

    apk_add_executable(${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME})
endif()
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/**.cpp)
target_sources(${PROJECT_NAME} PUBLIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC include)
set_target_properties(${PROJECT_NAME} PROPERTIES ANDROID_API ${CMAKE_ANDROID_API})
set_target_properties(${PROJECT_NAME} PROPERTIES ANDROID_API_MIN ${CMAKE_ANDROID_API_MIN})

check_cxx_compiler_flag(-fexperimental-library EXPER_LIB_SUPPORTED)
if (DEBUG)
    check_cxx_compiler_flag(-gcodeview CODEVIEW_SUPPORTED)
    check_cxx_compiler_flag(-gcolumn-info COLUMN_INFO_SUPPORTED)
    check_cxx_compiler_flag(-g CXX_DEBUG_SUPPORTED)
    check_cxx_compiler_flag(-fsanitize=address CXX_ASAN_SUPPORTED)
    check_cxx_compiler_flag(-fno-omit-frame-pointer NO_OMIT_FRAME_POINTER_SUPPORTED)
endif()

if (DEBUG)
    check_linker_flag(CXX -fuse-ld=lld USE_LLD_SUPPORTED)
    check_linker_flag(CXX -g LD_DEBUG_SUPPORTED)
    check_linker_flag(CXX -fsanitize=address LD_ASAN_SUPPORTED)
    check_linker_flag(CXX -Wl,--pdb= PDB_SUPPORTED)
endif()

if (DEBUG)
    target_compile_options(${PROJECT_NAME} PUBLIC
        $<$<BOOL:${CXX_DEBUG_SUPPORTED}>:-g>
        $<$<BOOL:${CXX_ASAN_SUPPORTED}>:-fsanitize=address>
        $<$<BOOL:${CODEVIEW_SUPPORTED}>:-gcodeview>
        $<$<BOOL:${COLUMN_INFO_SUPPORTED}>:-gcolumn-info>
        $<$<BOOL:${NO_OMIT_FRAME_POINTER_SUPPORTED}>:-fno-omit-frame-pointer>
    )
    target_link_options(${PROJECT_NAME} PUBLIC
        $<$<BOOL:${USE_LLD_SUPPORTED}>:-fuse-ld=lld>
        $<$<BOOL:${LD_DEBUG_SUPPORTED}>:-g>
        $<$<BOOL:${CXX_ASAN_SUPPORTED}>:-fsanitize=address>
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

# https://github.com/libcpr/cpr/issues/333#issuecomment-2425104338
if(ANDROID)
    set(CURL_USE_LIBPSL OFF)
    set(CPR_FORCE_OPENSSL_BACKEND ON)
    find_package(openssl REQUIRED CONFIG)
    # get_target_property(OPENSSL_CRYPTO_LIBRARY openssl::crypto IMPORTED_LOCATION)
    set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_CRYPTO_LIBRARY} CACHE INTERNAL "")

    # get_target_property(OPENSSL_SSL_LIBRARY openssl::ssl IMPORTED_LOCATION)
    set(OPENSSL_SSL_LIBRARY ${OPENSSL_SSL_LIBRARY} CACHE INTERNAL "")

    # get_target_property(OPENSSL_INCLUDE_DIR openssl::ssl INTERFACE_INCLUDE_DIRECTORIES)
    set(OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR} CACHE INTERNAL "")
else()
    set(CPR_USE_SYSTEM_LIB_PSL ON) # otherwise CPR will try building psl

    find_package(openssl QUIET)
    find_package(Libpsl QUIET)

    set(CURL_USE_LIBPSL Libpsl_FOUND)
    set(CPR_FORCE_OPENSSL_BACKEND openssl_FOUND)
endif()

add_subdirectory(submodules/SDL3)
add_subdirectory(submodules/SDL3_image)
add_subdirectory(submodules/SDL3_ttf)
add_subdirectory(submodules/discord-rpc)
add_subdirectory(submodules/cpr)

find_package(fmt QUIET)
if (NOT fmt_FOUND)
    add_subdirectory(submodules/fmt)
endif()

find_package(glaze QUIET)
if (NOT glaze_FOUND)
    add_subdirectory(submodules/glaze)
endif()

target_link_libraries(${PROJECT_NAME}
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

if (ANDROID)
    find_program(SED sed REQUIRED)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${SED} -i 's/\\r//g' ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_apk/gradlew*
        COMMENT "Removing carriage returns from gradle file"
    )
else()
    copy_libraries_recurse(${PROJECT_NAME})

    if (BUILD_SHARED_LIBS)
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${PROJECT_NAME}>
                    ${CMAKE_CURRENT_BINARY_DIR}/bin/$<TARGET_FILE_NAME:${PROJECT_NAME}>
            COMMENT "Copying $<TARGET_FILE_NAME:${PROJECT_NAME}> to build/bin"
        )
    endif()

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
endif()

set_property(
    DIRECTORY
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/bin"
)