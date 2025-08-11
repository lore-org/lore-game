include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)
include(CheckCXXSourceCompiles)

cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ----- Options -----

set(CMAKE_ANDROID_ARCH_ABI arm64-v8a CACHE STRING "What architecture to compile the Android application for")

set(CMAKE_ANDROID_API 33 CACHE STRING "Target Android API version. Default is Android 13.")
set(CMAKE_ANDROID_API_MIN 24 CACHE STRING "Minimum Android API version. Default is Android 7.0.")

set(BUILD_SHARED_LIBS ON CACHE STRING "Build .dlls, .so, and .dylibs.")

project("Lore Game" LANGUAGES C CXX VERSION 0.0.1)

set(APP_NAMESPACE "com.furred.${PROJECT_NAME}" CACHE STRING "The namespace given to the application.")
set(APP_NAME "${PROJECT_NAME}" CACHE STRING "The name given to the application.")

set(ANDROID_SPLASH_BACKGROUND_COLOR "#FFFFFF" CACHE STRING "The background colour for the Android splash screen.")

# -------------------

# Non-configurable variables

if(WIN32)
    enable_language(RC)
endif()

set(ARM_ARCHS arm arm64 aarch64)
if (CMAKE_SYSTEM_PROCESSOR IN_LIST ARM_ARCHS)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(ANDROID_ABI ${CMAKE_ANDROID_ARCH_ABI} CACHE STRING "" FORCE) # SDL relies on ANDROID_ABI instead of CMAKE_ANDROID_ARCH_ABI

# sanitise project name
string(TOLOWER "${PROJECT_NAME}" SANITISED_PROJECT_NAME)
string(REGEX REPLACE "[^a-zA-Z0-9_\\.+-]" "_" SANITISED_PROJECT_NAME "${SANITISED_PROJECT_NAME}")

# sanitise name
string(TOLOWER "${APP_NAME}" SANITISED_APP_NAME)
string(REGEX REPLACE "[^a-zA-Z0-9_\\.+-]" "_" SANITISED_APP_NAME "${SANITISED_APP_NAME}")

# sanitise namespace
string(TOLOWER "${APP_NAMESPACE}" SANITISED_APP_NAMESPACE)
string(REGEX REPLACE "[^a-zA-Z0-9_\\$\\.]" "_" SANITISED_APP_NAMESPACE "${SANITISED_APP_NAMESPACE}")

math(EXPR APP_VERSION_CODE "((${PROJECT_VERSION_MAJOR}+0) * 50000000) + ((${PROJECT_VERSION_MINOR}+0) * 1000000) + ((${PROJECT_VERSION_PATCH}+0) * 10000) + ((${PROJECT_VERSION_TWEAK}+0) * 1)")

if (ANDROID)
    find_program(GRADLE gradle REQUIRED)
endif()

file(REAL_PATH "~" HOME_DIR EXPAND_TILDE)
file(TO_CMAKE_PATH ${HOME_DIR} HOME_DIR)

if (BUILD_SHARED_LIBS)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX} ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

find_program(COMPILER_LAUNCHER sccache)
if (NOT COMPILER_LAUNCHER)
    find_program(COMPILER_LAUNCHER ccache)
endif()
if (COMPILER_LAUNCHER)
    set(CMAKE_C_COMPILER_LAUNCHER ${COMPILER_LAUNCHER} CACHE INTERNAL "")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${COMPILER_LAUNCHER} CACHE INTERNAL "")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(DEBUG ON)
else()
    set(DEBUG OFF)
endif()

set(DEBUG "${DEBUG}" CACHE BOOL "Whether to build debug libraries or not")

# Setup Python virtual env

find_program(PYTHON python3 py python REQUIRED)
set(PYTHON_VENV_DIR ${CMAKE_BINARY_DIR}/pyvenv)

find_program(
    VIRTUAL_PYTHON python3 py python
    NO_DEFAULT_PATH
    PATHS ${PYTHON_VENV_DIR}/bin ${PYTHON_VENV_DIR}/Scripts
)
if (NOT VIRTUAL_PYTHON)
    message(STATUS "Creating ${PYTHON_VENV_DIR}")
    execute_process(
        COMMAND ${PYTHON} -m venv ${PYTHON_VENV_DIR}
    )

    find_program(
        VIRTUAL_PYTHON python3 py python
        REQUIRED
        NO_DEFAULT_PATH
        PATHS ${PYTHON_VENV_DIR}/bin ${PYTHON_VENV_DIR}/Scripts
    )
endif()

execute_process(COMMAND ${VIRTUAL_PYTHON} -m pip install -r ${CMAKE_SOURCE_DIR}/python/requirements.txt)

# Env detection

if (NOT DEFINED ANDROID_NDK)
    if (ANDROID AND NOT DEFINED ENV{ANDROID_NDK}) # don't error on non-android build platforms
        message(FATAL_ERROR "ANDROID_NDK must be defined as a cmake variable or environment variable.")
    endif()
    set(ANDROID_NDK $ENV{ANDROID_NDK} CACHE STRING "") # libwebp relies on a cmake ANDROID_NDK instead of an env ANDROID_NDK
endif()

if (NOT DEFINED JAVA_HOME)
    if (ANDROID AND NOT DEFINED ENV{JAVA_HOME}) # don't error on non-android build platforms
        message(FATAL_ERROR "JAVA_HOME must be defined as a cmake variable or environment variable.")
    endif()
    set(JAVA_HOME $ENV{JAVA_HOME} CACHE STRING "")
endif()

# Getting pre-installed libraries

# TODO - add windows support
function(get_lib_arch LIB RESULT_VAR)
    find_program(FILE file)
    find_program(SED sed)

    if (FILE AND SED)
        set(REAL_LIB ${LIB})
        file(REAL_PATH ${LIB} REAL_LIB)

        execute_process(
            COMMAND ${FILE} ${REAL_LIB}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE OUTPUT
        )
        
        string(REGEX MATCH ", ([^,]+)," _ "${OUTPUT}") # pull arch from string
        string(STRIP "${CMAKE_MATCH_1}" FULL_ARCH)

        string(REGEX MATCH "[^ ]+$" OUTPUT "${FULL_ARCH}") # get isolated arch at end of string
        string(TOLOWER "${OUTPUT}" OUTPUT)

        set("${RESULT_VAR}" "${OUTPUT}" PARENT_SCOPE)
    endif()
endfunction()

function(add_libs_from_path PATH)
    # do NOT configure_depends, this will take 10 million years

    file(TO_CMAKE_PATH ${PATH} TEMP_PATH)
    file(GLOB TEMP_SOURCES ${TEMP_PATH})
    list(APPEND CMAKE_PREFIX_PATH ${TEMP_SOURCES})

    foreach (SOURCE IN LISTS TEMP_SOURCES)
        file(GLOB SOURCE_LIBS ${SOURCE}/lib ${SOURCE}/lib/* ${SOURCE}/*/lib)

        foreach (LIB_PATH IN LISTS SOURCE_LIBS)
            cmake_path(GET LIB_PATH FILENAME CURRENT_DIR)
            if (
                NOT IS_DIRECTORY ${LIB_PATH} OR
                # These are paths from brew and do not contain libraries
                ${CURRENT_DIR} STREQUAL "pkgconfig" OR
                ${CURRENT_DIR} STREQUAL "cmake" 
            )
                continue()
            endif()
            if (EXISTS ${LIB_PATH}/pkgconfig)
                list(APPEND CMAKE_PREFIX_PATH ${LIB_PATH}/pkgconfig)
            endif()

            cmake_path(GET LIB_PATH PARENT_PATH LIB_PARENT)
            if (${LIB_PARENT} STREQUAL ${SOURCE})
                set(LIB_RELATIVE lib)
            else()
                cmake_path(GET LIB_PARENT PARENT_PATH LIB_PARENT)
                cmake_path(RELATIVE_PATH LIB_PATH BASE_DIRECTORY ${LIB_PARENT} OUTPUT_VARIABLE LIB_RELATIVE)
            endif()
            string(REPLACE "lib" "include" INCLUDE_RELATIVE ${LIB_RELATIVE})
            set(INCLUDE_PATH ${SOURCE}/${INCLUDE_RELATIVE})

            if (NOT EXISTS ${SOURCE}/${INCLUDE_RELATIVE})
                continue()
            endif()

            list(GET CMAKE_FIND_LIBRARY_SUFFIXES 0 WANTED_LIBRARY_SUFFIX)
            message(STATUS "CMAKE_FIND_LIBRARY_SUFFIXES: ${CMAKE_FIND_LIBRARY_SUFFIXES}")
            message(STATUS "WANTED_LIBRARY_SUFFIX:       ${WANTED_LIBRARY_SUFFIX}")
            file(GLOB LIBRARY_FILES ${LIB_PATH}/*${WANTED_LIBRARY_SUFFIX})

            foreach (LIBRARY IN LISTS LIBRARY_FILES)
                # quit early if arch doesn't exist or doesn't match target
                get_lib_arch(${LIBRARY} ARCH)
                if (
                    ARCH STREQUAL "" OR
                    NOT ARCH STREQUAL ${CMAKE_SYSTEM_PROCESSOR}
                )
                    continue()
                endif()

                cmake_path(GET LIBRARY FILENAME LIBRARY_NAME)
                string(REGEX REPLACE "[^a-zA-Z0-9_\\.\\+-]" "" LIBRARY_NAME "${LIBRARY_NAME}") # sanitize library

                cmake_path(GET LIBRARY_NAME STEM STRIPPED_LIBRARY_NAME)

                string(REGEX REPLACE "^lib" "" BARE_STRIPPED_LIBRARY_NAME ${STRIPPED_LIBRARY_NAME})

                set(LIBRARY_NAMES
                    ${LIBRARY_NAME}
                    ${STRIPPED_LIBRARY_NAME}
                    ${BARE_STRIPPED_LIBRARY_NAME}
                )
                
                message(STATUS "Found valid library \"${STRIPPED_LIBRARY_NAME}\": ${SOURCE}")

                foreach (LIBRARY_NAME IN LISTS LIBRARY_NAMES)
                    string(TOLOWER ${LIBRARY_NAME} LOWER_LIBRARY_NAME)
                    string(TOUPPER ${LIBRARY_NAME} UPPER_LIBRARY_NAME)
                    set(LIBRARY_NAMES
                        ${LOWER_LIBRARY_NAME}
                        ${LIBRARY_NAME}
                        ${UPPER_LIBRARY_NAME}
                    )

                    foreach (LIBRARY_NAME IN LISTS LIBRARY_NAMES)
                        set(LIBRARY_NAMES
                            ${LIBRARY_NAME}
                            ${LIBRARY_NAME}::${LIBRARY_NAME}
                        )

                        foreach (LIBRARY_NAME IN LISTS LIBRARY_NAMES)
                            if (NOT TARGET "${LIBRARY_NAME}")
                                if (BUILD_SHARED_LIBS)
                                    add_library("${LIBRARY_NAME}" SHARED IMPORTED)
                                else()
                                    add_library("${LIBRARY_NAME}" STATIC IMPORTED)
                                endif()
                            
                                set_target_properties("${LIBRARY_NAME}" PROPERTIES
                                    IMPORTED_LOCATION ${LIBRARY}
                                )

                                target_include_directories("${LIBRARY_NAME}" INTERFACE ${INCLUDE_PATH})
                            endif()

                            string(REPLACE "::" "_" SNAKE_LIBRARY_NAME "${LIBRARY_NAME}")
                            
                            set("${LIBRARY_NAME}_LIBRARY" ${LIBRARY} PARENT_SCOPE)
                            set("${SNAKE_LIBRARY_NAME}_LIBRARY" ${LIBRARY} PARENT_SCOPE)

                            set("${LIBRARY_NAME}_INCLUDE_DIR" ${INCLUDE_PATH} PARENT_SCOPE)
                            set("${SNAKE_LIBRARY_NAME}_INCLUDE_DIR" ${INCLUDE_PATH} PARENT_SCOPE)
                        endforeach()
                    endforeach()
                endforeach()
            endforeach()
        endforeach()
    endforeach()
endfunction()

message("Looking for libraries matching arch '${CMAKE_SYSTEM_PROCESSOR}'")

# linux libraries
if (EXISTS /usr/lib)
    message("Looking for libraries: /usr")
    add_libs_from_path(/usr)
endif()

# vcpkg libraries
if (DEFINED ENV{VCPKG_ROOT})
    message("Looking for libraries: $ENV{VCPKG_ROOT}")
    add_libs_from_path($ENV{VCPKG_ROOT}/packages/*)
endif()

# Android libraries
if (DEFINED ANDROID_NDK)
    message("Looking for libraries: ${ANDROID_NDK}")
    add_libs_from_path(${ANDROID_NDK}/toolchains/llvm/prebuilt/*)
    add_libs_from_path(${ANDROID_NDK}/toolchains/llvm/prebuilt/*/sysroot/usr)
endif()

# Brew libraries
find_program(BREW brew)
if (BREW)
    execute_process(
        COMMAND ${BREW} --cellar
        OUTPUT_VARIABLE BREW_CELLAR
    )
    string(STRIP ${BREW_CELLAR} BREW_CELLAR)
    message("Looking for libraries: ${BREW_CELLAR}")
    add_libs_from_path(${BREW_CELLAR}/*/*)
endif()

# Setup Android-specific dependencies

if (ANDROID)
    # openssl

    set(OPENSSL_ROOT_PATH ${CMAKE_SOURCE_DIR}/submodules/android_openssl)
    
    if (CMAKE_BUILD_TYPE STREQUAL "Debug") # Specifically debug
        string(APPEND OPENSSL_ROOT_PATH "/no-asm")
    endif()

    set(OPENSSL_LIB_DIR ${OPENSSL_ROOT_PATH}/ssl_3/${CMAKE_ANDROID_ARCH_ABI})
    set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_PATH}/ssl_3/include)

    list(APPEND CMAKE_PREFIX_PATH ${OPENSSL_LIB_DIR})

    list(GET CMAKE_FIND_LIBRARY_SUFFIXES 0 WANTED_LIBRARY_SUFFIX)

    if (DEFINED crypto_LIBRARY)
        set(crypto ${crypto_LIBRARY})
    else()
        set(crypto ${OPENSSL_LIB_DIR}/libcrypto${WANTED_LIBRARY_SUFFIX})
    endif()

    if (DEFINED ssl_LIBRARY)
        set(ssl ${ssl_LIBRARY})
    else()
        set(ssl ${OPENSSL_LIB_DIR}/libssl${WANTED_LIBRARY_SUFFIX})
    endif()

    # Eliminate symbolic links
    file(REAL_PATH ${crypto} crypto)
    file(REAL_PATH ${ssl} ssl)
    file(REAL_PATH ${OPENSSL_INCLUDE_DIR} OPENSSL_INCLUDE_DIR)

    if (NOT EXISTS ${crypto})
        message(FATAL_ERROR "Could not find OpenSSL::Crypto (looking for ${crypto})")
    endif()
    message("Found OpenSSL::Crypto: ${crypto}")

    if (NOT EXISTS ${ssl})
        message(FATAL_ERROR "Could not find OpenSSL::SSL (looking for ${ssl})")
    endif()
    message("Found OpenSSL::SSL: ${ssl}")

    function(add_imported_library target type)
        if (NOT TARGET ${target})
            add_library(${target} ${type} IMPORTED)
        endif()
    endfunction()

    function(add_or_edit_library_alias alias target)
        if (TARGET ${alias})
            set_target_properties(${alias} PROPERTIES
                ALIASED_TARGET ${target}
            )
        else()
            add_library(${alias} ALIAS ${target})
        endif()
    endfunction()


    if (BUILD_SHARED_LIBS)
        add_imported_library(libcrypto SHARED)
        add_imported_library(libssl SHARED)
    else()
        add_imported_library(libcrypto STATIC)
        add_imported_library(libssl STATIC)
    endif()

    set_target_properties(libcrypto PROPERTIES
        IMPORTED_LOCATION ${crypto}
    )
    set_target_properties(libssl PROPERTIES
        IMPORTED_LOCATION ${ssl}
    )

    add_or_edit_library_alias(OpenSSL::Crypto libcrypto)
    add_or_edit_library_alias(OpenSSL::SSL libssl)

    target_include_directories(libcrypto INTERFACE ${OPENSSL_INCLUDE_DIR})
    target_include_directories(libssl INTERFACE ${OPENSSL_INCLUDE_DIR})

    # https://github.com/libcpr/cpr/issues/333#issuecomment-2425104338
    set(OPENSSL_SSL_LIBRARY ${ssl} CACHE INTERNAL "")
    set(OPENSSL_CRYPTO_LIBRARY ${crypto} CACHE INTERNAL "")
    set(OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR} CACHE INTERNAL "")
endif()

# Init project

if (ANDROID)
    add_library("${SANITISED_PROJECT_NAME}" SHARED)
else()
    add_executable("${SANITISED_PROJECT_NAME}")
    set_target_properties("${SANITISED_PROJECT_NAME}" PROPERTIES OUTPUT_NAME "${APP_NAME}")
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
target_sources("${SANITISED_PROJECT_NAME}" PUBLIC ${SOURCES})

target_include_directories("${SANITISED_PROJECT_NAME}" PUBLIC include)
target_include_directories("${SANITISED_PROJECT_NAME}" PUBLIC ${CMAKE_BINARY_DIR}/include)

if (WIN32 AND EXISTS ${CMAKE_BINARY_DIR}/assets/app.rc)
    target_sources("${SANITISED_PROJECT_NAME}" PUBLIC ${CMAKE_BINARY_DIR}/assets/app.rc)
endif()

if (ANDROID)
    # set(NATIVE_APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
    # target_sources("${SANITISED_PROJECT_NAME}" PUBLIC ${NATIVE_APP_GLUE_DIR}/android_native_app_glue.c)
    # target_include_directories("${SANITISED_PROJECT_NAME}" PUBLIC ${NATIVE_APP_GLUE_DIR})

    file(GLOB ANDROID_LIBRARIES ${ANDROID_NDK}/toolchains/llvm/prebuilt/*/sysroot/usr/include)
    target_include_directories("${SANITISED_PROJECT_NAME}" PUBLIC ${ANDROID_LIBRARIES})
    
    set_target_properties("${SANITISED_PROJECT_NAME}" PROPERTIES ANDROID_API ${CMAKE_ANDROID_API})
    set_target_properties("${SANITISED_PROJECT_NAME}" PROPERTIES ANDROID_API_MIN ${CMAKE_ANDROID_API_MIN})
endif()

set_target_properties("${SANITISED_PROJECT_NAME}" PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_ICON_FILE "${CMAKE_BINARY_DIR}/assets/icon.icns"
)

if (NOT DEBUG)
    set_target_properties("${SANITISED_PROJECT_NAME}" PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

target_compile_definitions("${SANITISED_PROJECT_NAME}" PUBLIC
    "__APP_NAMESPACE__=\"${APP_NAMESPACE}\""
    "__APP_NAME__=\"${APP_NAME}\""
    "__APP_VERSION__=\"${PROJECT_VERSION}\""
    "__APP_VERSION_CODE__=\"${APP_VERSION_CODE}\""
)

# Not exactly favourable but the current multiplatform logging system generates a bunch of errors
target_compile_options("${SANITISED_PROJECT_NAME}" PRIVATE -Wno-format-nonliteral)

# Generate resources

file(REMOVE_RECURSE
    ${CMAKE_BINARY_DIR}/android
    ${CMAKE_BINARY_DIR}/include
)

file(READ config.json CONFIG_FILE)
configure_file(include/engine/config.hpp.in include/engine/config.hpp)

execute_process(COMMAND ${VIRTUAL_PYTHON} ${CMAKE_SOURCE_DIR}/python/icon_generator.py --crop-type longest --build-dir ${CMAKE_BINARY_DIR} --resources-dir ${CMAKE_SOURCE_DIR}/resources)

# Linker/Compiler checking

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

target_compile_options("${SANITISED_PROJECT_NAME}" PUBLIC
    $<$<BOOL:${EXPER_LIB_SUPPORTED}>:-fexperimental-library>
)

if (DEBUG)
    target_compile_options("${SANITISED_PROJECT_NAME}" PUBLIC
        $<$<BOOL:${CXX_DEBUG_SUPPORTED}>:-g>
        $<$<BOOL:${CXX_ASAN_SUPPORTED}>:-fsanitize=address>
        $<$<BOOL:${CODEVIEW_SUPPORTED}>:-gcodeview>
        $<$<BOOL:${COLUMN_INFO_SUPPORTED}>:-gcolumn-info>
        $<$<BOOL:${NO_OMIT_FRAME_POINTER_SUPPORTED}>:-fno-omit-frame-pointer>
    )
    target_link_options("${SANITISED_PROJECT_NAME}" PUBLIC
        $<$<BOOL:${USE_LLD_SUPPORTED}>:-fuse-ld=lld>
        $<$<BOOL:${LD_DEBUG_SUPPORTED}>:-g>
        $<$<BOOL:${CXX_ASAN_SUPPORTED}>:-fsanitize=address>
        $<$<BOOL:${PDB_SUPPORTED}>:-Wl,--pdb=>
    )
endif()

check_cxx_source_compiles("
    #include <execution>
    #include <vector>

    int main(void) {
        std::vector<char> v = {};
        std::sort(std::execution::par, v.begin(), v.end());
    }
" HAS_PAR)

check_cxx_source_compiles("
    #include <execution>
    #include <vector>

    int main(void) {
        std::vector<char> v = {};
        std::sort(std::execution::par_unseq, v.begin(), v.end());
    }
" HAS_PAR_UNSEQ)

target_compile_definitions("${SANITISED_PROJECT_NAME}" PUBLIC
    $<$<BOOL:${HAS_PAR_UNSEQ}>:HAS_PAR>
    $<$<BOOL:${HAS_PAR_UNSEQ}>:HAS_PAR_UNSEQ>
)

# Configure submodules

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
set(SDLIMAGE_JXL ON)

set(SDLTTF_VENDORED ON)

set(CPR_USE_SYSTEM_LIB_PSL ON) # otherwise CPR will try building psl
set(CURL_USE_LIBPSL OFF)

function(use_found_or_vendored_with_output LIB PATH FOUND_VAR)
    set(${FOUND_VAR} FALSE PARENT_SCOPE)
    find_package(${LIB} QUIET)
    if (${LIB}_FOUND OR TARGET ${LIB})
        message(STATUS "Found Library: ${LIB}")
        set(${FOUND_VAR} TRUE PARENT_SCOPE)
    else()
        message(STATUS "Could not find Library: ${LIB}. Compiling...")
        add_subdirectory(${PATH})
    endif()
endfunction()

function(use_found_or_vendored LIB PATH)
    use_found_or_vendored_with_output(${LIB} ${PATH} _)
endfunction()

function(set_vendored_if_found LIB TOGGLE_VAR)
    find_package(${LIB} QUIET)
    if (${LIB}_FOUND OR TARGET ${LIB})
        message(STATUS "Found Library: ${LIB}")
        set(${TOGGLE_VAR} OFF CACHE BOOL "" FORCE)
    else()
        set(${TOGGLE_VAR} ON CACHE BOOL "" FORCE)
    endif()
endfunction()

set(TEMP_CMAKE_POLICY_VERSION_MINIMUM ${CMAKE_POLICY_VERSION_MINIMUM})
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set_vendored_if_found(zlibstatic SDLIMAGE_ZLIB_VENDORED)
set_vendored_if_found(dav1d SDLIMAGE_DAV1D_VENDORED)
set_vendored_if_found(aom SDLIMAGE_AOM_VENDORED)
set_vendored_if_found(avif SDLIMAGE_AVIF_VENDORED)
set_vendored_if_found(jpeg SDLIMAGE_JPG_VENDORED)
set_vendored_if_found(jxl_dec-static SDLIMAGE_JXL_VENDORED)
set_vendored_if_found(png_static SDLIMAGE_PNG_VENDORED)
set_vendored_if_found(tiff SDLIMAGE_TIF_VENDORED)
set_vendored_if_found(webp SDLIMAGE_WEBP_VENDORED)

set_vendored_if_found(harfbuzz SDLTTF_HARFBUZZ_VENDORED)
set_vendored_if_found(freetype SDLTTF_FREETYPE_VENDORED)
set_vendored_if_found(plutovg SDLTTF_PLUTOSVG_VENDORED)
set_vendored_if_found(plutosvg SDLTTF_PLUTOSVG_VENDORED)

use_found_or_vendored(SDL3::SDL3 submodules/SDL3)
use_found_or_vendored(SDL3_image::SDL3_image submodules/SDL3_image)
use_found_or_vendored(SDL3_image::SDL3_ttf submodules/SDL3_ttf)
use_found_or_vendored(discord-rpc submodules/discord-rpc)
use_found_or_vendored(cpr submodules/cpr)
use_found_or_vendored(fmt submodules/fmt)
use_found_or_vendored(glaze submodules/glaze)

target_compile_options(cpr PRIVATE -Wno-error) # CPR has some warning on arm platforms due to the `long` type
# With how we use openssl on android, it needs to be linked manually
if (TARGET OpenSSL::SSL)
    target_link_libraries(cpr PUBLIC OpenSSL::SSL)
endif()
if (TARGET OpenSSL::Crypto)
    target_link_libraries(cpr PUBLIC OpenSSL::Crypto)
endif()

set(CMAKE_POLICY_VERSION_MINIMUM ${TEMP_CMAKE_POLICY_VERSION_MINIMUM})

# Add libraries to project

set(LIBRARIES
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
    discord-rpc
    cpr
    fmt
    glaze::glaze
)

target_link_libraries("${SANITISED_PROJECT_NAME}" ${LIBRARIES})

# Copy resources and binaries after build

function(get_lib_copy_dir VAR)
    if (ANDROID)
        set(${VAR} android/app/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI} PARENT_SCOPE)
    else()
        set(${VAR} bin PARENT_SCOPE)
    endif()
endfunction()

function(get_res_copy_dir VAR)
    if (ANDROID)
        set(${VAR} android/app/src/main/assets PARENT_SCOPE)
    else()
        set(${VAR} bin PARENT_SCOPE)
    endif()
endfunction()


function(copy_target target)
    get_lib_copy_dir(DIR)
    add_custom_command(
        TARGET "${SANITISED_PROJECT_NAME}"
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:${target}>
                ${CMAKE_BINARY_DIR}/${DIR}/$<TARGET_FILE_NAME:${target}>
        COMMENT "Copying $<TARGET_FILE_NAME:${target}> to build/${DIR}"
    )
endfunction()

function(copy_libraries_recurse target)
    get_target_property(libraries ${target} LINK_LIBRARIES)
    foreach(library IN LISTS libraries)
        if (
            library STREQUAL "HarfBuzz::HarfBuzz" OR # Harfbuzz links to itself somehow
            NOT TARGET ${library}
        )
            continue()
        endif()

        copy_libraries_recurse(${library})

        get_target_property(target_type ${library} TYPE)
        if (NOT target_type STREQUAL "SHARED_LIBRARY")
            continue()
        endif()

        copy_target(${library})
    endforeach()
endfunction()

file(GLOB_RECURSE DEBUG_BINARIES CONFIGURE_DEPENDS ${CMAKE_BINARY_DIR}/*.pdb)
add_custom_command(
    TARGET "${SANITISED_PROJECT_NAME}"
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm
            -rf
            ${CMAKE_BINARY_DIR}/bin
            "${DEBUG_BINARIES}"
    COMMENT "Cleaning extra artifacts"
    COMMAND_EXPAND_LISTS
)

copy_target("${SANITISED_PROJECT_NAME}")
if (BUILD_SHARED_LIBS)
    copy_libraries_recurse("${SANITISED_PROJECT_NAME}")
endif()

if (NOT ANDROID)
    if (EXISTS ${CMAKE_BINARY_DIR}/${APP_NAME}.pdb)
        add_custom_command(
            TARGET "${SANITISED_PROJECT_NAME}"
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_BINARY_DIR}/${APP_NAME}.pdb
                    ${CMAKE_BINARY_DIR}/bin/${APP_NAME}.pdb
            COMMENT "Copying ${APP_NAME}.pdb to build/bin"
        )
    endif()
endif()

get_res_copy_dir(RES_DIR)
add_custom_command(
    TARGET "${SANITISED_PROJECT_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            ${CMAKE_SOURCE_DIR}/resources
            ${CMAKE_BINARY_DIR}/${RES_DIR}/resources
    COMMENT "Copying resources to build/${RES_DIR}"
)
if (ANDROID)
    file(GLOB_RECURSE ANDROID_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/android/**)
    foreach (ANDROID_SOURCE IN LISTS ANDROID_SOURCES)
        cmake_path(GET ANDROID_SOURCE FILENAME SOURCE_NAME)

        cmake_path(GET ANDROID_SOURCE PARENT_PATH SOURCE_DIR)
        cmake_path(RELATIVE_PATH SOURCE_DIR)

        if (${ANDROID_SOURCE} MATCHES ".in$")
            string(REGEX REPLACE "\\.[^.]*$" "" SOURCE_NAME ${SOURCE_NAME})
        endif()

        configure_file(${ANDROID_SOURCE} ${CMAKE_BINARY_DIR}/${SOURCE_DIR}/${SOURCE_NAME})
    endforeach()
endif()

if (ANDROID)
    add_custom_command(
        TARGET "${SANITISED_PROJECT_NAME}"
        POST_BUILD
        COMMAND ${GRADLE} -p ${CMAKE_BINARY_DIR}/android $<IF:$<BOOL:DEBUG>,assembleDebug,assembleRelease>
        VERBATIM
        USES_TERMINAL
    )
endif()