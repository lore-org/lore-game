include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
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

option(IMAGE_DEFAULTS "Support loading JPEG, PNG, and WEBP images" ON)
option(IMAGE_EXTRAS "Support loading AVIF, JPEG, PNG, SVG, and WEBP images" OFF)
option(IMAGE_ALL "Support loading all images supported by SDL3_image" OFF)

# for more options, set the values below:
# IMAGE_AVIF
# IMAGE_BMP
# IMAGE_GIF
# IMAGE_JPG
# IMAGE_LBM
# IMAGE_PCX
# IMAGE_PNG
# IMAGE_PNM
# IMAGE_QOI
# IMAGE_SVG
# IMAGE_TGA
# IMAGE_TIF
# IMAGE_WEBP
# IMAGE_XCF
# IMAGE_XPM
# IMAGE_XV

set(CPM_VERSION 0.42.0 CACHE STRING "Version of CPM to download")
set(CPM_HASH eed8973c5595c72d9ba785a5472e9750 CACHE STRING "Hash of the specific version of CPM")

# -------------------

# Non-configurable variables

if(WIN32)
    enable_language(RC)
endif()

set(ARM_ARCHS arm arm64 aarch64)
if (NORMALISED_CMAKE_SYSTEM_PROCESSOR IN_LIST ARM_ARCHS)
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

function(set_if_empty VARIABLE VALUE)
    if (NOT DEFINED "${VARIABLE}")
        set("${VARIABLE}" "${VALUE}" PARENT_SCOPE)
    endif()
endfunction()

if (IMAGE_DEFAULTS)
    set_if_empty(IMAGE_JPG  ON)
    set_if_empty(IMAGE_PNG  ON)
    set_if_empty(IMAGE_WEBP ON)
endif()

if (IMAGE_EXTRAS)
    set_if_empty(IMAGE_AVIF ON)
    set_if_empty(IMAGE_JPG  ON)
    set_if_empty(IMAGE_JXL  ON)
    set_if_empty(IMAGE_PNG  ON)
    set_if_empty(IMAGE_SVG  ON)
    set_if_empty(IMAGE_WEBP ON)
endif()

if (IMAGE_ALL)
    set_if_empty(IMAGE_AVIF ON)
    set_if_empty(IMAGE_BMP  ON)
    set_if_empty(IMAGE_GIF  ON)
    set_if_empty(IMAGE_JPG  ON)
    set_if_empty(IMAGE_JXL  ON)
    set_if_empty(IMAGE_LBM  ON)
    set_if_empty(IMAGE_PCX  ON)
    set_if_empty(IMAGE_PNG  ON)
    set_if_empty(IMAGE_PNM  ON)
    set_if_empty(IMAGE_QOI  ON)
    set_if_empty(IMAGE_SVG  ON)
    set_if_empty(IMAGE_TGA  ON)
    set_if_empty(IMAGE_TIF  ON)
    set_if_empty(IMAGE_WEBP ON)
    set_if_empty(IMAGE_XCF  ON)
    set_if_empty(IMAGE_XPM  ON)
    set_if_empty(IMAGE_XV   ON)
endif()

set_if_empty(IMAGE_AVIF OFF)
set_if_empty(IMAGE_BMP  OFF)
set_if_empty(IMAGE_GIF  OFF)
set_if_empty(IMAGE_JPG  OFF)
set_if_empty(IMAGE_JXL  OFF)
set_if_empty(IMAGE_LBM  OFF)
set_if_empty(IMAGE_PCX  OFF)
set_if_empty(IMAGE_PNG  OFF)
set_if_empty(IMAGE_PNM  OFF)
set_if_empty(IMAGE_QOI  OFF)
set_if_empty(IMAGE_SVG  OFF)
set_if_empty(IMAGE_TGA  OFF)
set_if_empty(IMAGE_TIF  OFF)
set_if_empty(IMAGE_WEBP OFF)
set_if_empty(IMAGE_XCF  OFF)
set_if_empty(IMAGE_XPM  OFF)
set_if_empty(IMAGE_XV   OFF)

set(LIBCXX_ENABLE_SHARED OFF)
set(LIBCXX_ENABLE_STATIC ON)

# Download CPM

set_if_empty(CPM_VERSION 0.42.0)
set_if_empty(CPM_HASH eed8973c5595c72d9ba785a5472e9750)

set(_cpm_file "${CMAKE_BINARY_DIR}/get_cpm.cmake")

if (EXISTS "${_cpm_file}")
    file(MD5 "${_cpm_file}" CURRENT_CPM_HASH)

    if (CURRENT_CPM_HASH STREQUAL CPM_HASH)
        set(CURRENT_CPM_VALID TRUE)
        message(STATUS "Found existing CPM v${CPM_VERSION} (MD5: ${CPM_HASH})")
    endif()
endif()

if (NOT CURRENT_CPM_VALID)
    message(STATUS "Downloading CPM v${CPM_VERSION} (MD5: ${CPM_HASH})")
    file(DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_VERSION}/get_cpm.cmake"
        "${_cpm_file}"
        EXPECTED_HASH "MD5=${CPM_HASH}"
        SHOW_PROGRESS
    )
endif()

include("${_cpm_file}")

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

function(normalise_arch ARCH RESULT_VAR)
    string(TOLOWER "${ARCH}" ARCH)
    string(REGEX REPLACE "[^a-zA-Z0-9]" "_" ARCH "${ARCH}")

    set(x64_ARCH_LIST x64
        x86_64
        amd64
        hurd-amd64
        kfreebsd-amd64
    )
    set(x32_ARCH_LIST x32
        x86
    )
    set(arm_ARCH_LIST arm
        armel
        armhf
    )
    set(arm64_ARCH_LIST arm64
        aarch64
    )

    set(ARCH_LISTS
        x64_ARCH_LIST
        x32_ARCH_LIST
        arm_ARCH_LIST
        arm64_ARCH_LIST
    )
    foreach(ARCH_LIST IN LISTS ARCH_LISTS)
        if (ARCH IN_LIST "${ARCH_LIST}")
            list(GET "${ARCH_LIST}" 0 NORMALISED_ARCH)
            set(ARCH "${NORMALISED_ARCH}")
        endif()
    endforeach()

    set("${RESULT_VAR}" ${ARCH} PARENT_SCOPE)
endfunction()

normalise_arch("${CMAKE_SYSTEM_PROCESSOR}" NORMALISED_CMAKE_SYSTEM_PROCESSOR)

# TODO - add windows support
function(get_lib_arch LIB RESULT_VAR)
    find_program(FILE file)

    if (FILE)
        set(REAL_LIB "${LIB}")
        file(REAL_PATH "${LIB}" REAL_LIB)

        execute_process(
            COMMAND ${FILE} "${REAL_LIB}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE OUTPUT
        )
        
        string(REGEX MATCH ", ([^,]+)," _ "${OUTPUT}") # pull arch from string
        string(STRIP "${CMAKE_MATCH_1}" FULL_ARCH)

        string(REGEX MATCH "[^ ]+$" OUTPUT "${FULL_ARCH}") # get isolated arch at end of string
        string(TOLOWER "${OUTPUT}" OUTPUT)

        normalise_arch("${OUTPUT}" NORMALISED_OUTPUT)

        set("${RESULT_VAR}" "${NORMALISED_OUTPUT}" PARENT_SCOPE)
    endif()
endfunction()

# function(add_libs_from_path PATH_GLOB)
#     # do NOT configure_depends, this will take 10 million years

#     file(TO_CMAKE_PATH "${PATH_GLOB}" TEMP_PATH)
#     file(GLOB TEMP_SOURCES "${TEMP_PATH}")
#     set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${TEMP_SOURCES}" PARENT_SCOPE)

#     foreach (SOURCE IN LISTS TEMP_SOURCES)
#         if (EXISTS "${SOURCE}/lib")
#             list(APPEND CMAKE_LIBRARY_PATH ${SOURCE}/lib)
#         endif()
#         if (EXISTS "${SOURCE}/include")
#             list(APPEND CMAKE_INCLUDE_PATH ${SOURCE}/include)
#         endif()
#     endforeach()
# endfunction()

# # Brew libraries
# find_program(BREW brew)
# if (BREW)
#     execute_process(
#         COMMAND ${BREW} --prefix
#         OUTPUT_VARIABLE BREW_PREFIX
#     )
#     string(STRIP ${BREW_PREFIX} BREW_PREFIX)

#     add_libs_from_path(${BREW_PREFIX})
# endif()

# # Android libraries
# if (DEFINED ANDROID_NDK)
#     add_libs_from_path(${ANDROID_NDK}/toolchains/llvm/prebuilt/*)
#     add_libs_from_path(${ANDROID_NDK}/toolchains/llvm/prebuilt/*/sysroot/usr)
# endif()

# # vcpkg libraries
# if (DEFINED ENV{VCPKG_ROOT})
#     add_libs_from_path($ENV{VCPKG_ROOT}/packages/*)
# endif()

# # linux libraries
# if (EXISTS /usr/lib)
#     add_libs_from_path(/usr)
# endif()

# Setup Android-specific dependencies

if (ANDROID)
    CPMAddPackage(
        NAME android_openssl
        GIT_REPOSITORY https://github.com/KDAB/android_openssl.git
        GIT_TAG 0025bbe48f69792f95e02c9289df0fae68c954d6
        DOWNLOAD_ONLY YES
    )

    # openssl

    set(OPENSSL_ROOT_PATH ${android_openssl_SOURCE_DIR})
    
    if (CMAKE_BUILD_TYPE STREQUAL "Debug") # Specifically debug
        string(APPEND OPENSSL_ROOT_PATH "/no-asm")
    endif()

    set(OPENSSL_LIB_DIR ${OPENSSL_ROOT_PATH}/ssl_3/${CMAKE_ANDROID_ARCH_ABI})
    set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_PATH}/ssl_3/include)

    list(APPEND CMAKE_PREFIX_PATH ${OPENSSL_LIB_DIR})

    if (BUILD_SHARED_LIBS)
        set(WANTED_LIBRARY_SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
    else()
        set(WANTED_LIBRARY_SUFFIX "${CMAKE_STATIC_LIBRARY_SUFFIX}")
    endif()

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
    message(STATUS "Found OpenSSL::Crypto: ${crypto}")

    if (NOT EXISTS ${ssl})
        message(FATAL_ERROR "Could not find OpenSSL::SSL (looking for ${ssl})")
    endif()
    message(STATUS "Found OpenSSL::SSL: ${ssl}")

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

    message(DEBUG "OPENSSL_SSL_LIBRARY:    ${OPENSSL_SSL_LIBRARY}")
    message(DEBUG "OPENSSL_CRYPTO_LIBRARY: ${OPENSSL_CRYPTO_LIBRARY}")
    message(DEBUG "OPENSSL_INCLUDE_DIR:    ${OPENSSL_INCLUDE_DIR}")
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

list(APPEND CXX_COMPILER_FLAGS
    "-Wa,-mbig-obj"
    "/bigobj"
    "-Wno-c++98-compat"
    "-fexperimental-library"
    "-Wno-error"
)

if (DEBUG)
    list(APPEND CXX_COMPILER_FLAGS
        "-gcodeview"
        "-gcolumn-info"
        "-g"
        "-fsanitize=address"
        "-fno-omit-frame-pointer"
    )
endif()

list(APPEND C_COMPILER_FLAGS

)

list(APPEND CXX_LINKER_FLAGS

)

if (DEBUG)
    list(APPEND CXX_LINKER_FLAGS
        "-fuse-ld=lld"
        "-g"
        "-fsanitize=address"
        "-Wl,--pdb="
    )
endif()

list(APPEND C_LINKER_FLAGS

)

macro(check_set_cxx_compiler_flag FLAG)
    string(REGEX REPLACE "[^a-zA-Z0-9_-]" "_" SANITISED_FLAG "${FLAG}")
    check_cxx_compiler_flag("${FLAG}" "SUPPORTS_CMP_${SANITISED_FLAG}")
endmacro()

macro(check_set_c_compiler_flag FLAG)
    string(REGEX REPLACE "[^a-zA-Z0-9_-]" "_" SANITISED_FLAG "${FLAG}")
    check_c_compiler_flag("${FLAG}" "SUPPORTS_CMP_${SANITISED_FLAG}")
endmacro()


macro(set_compile_flag_if_supported FLAG)
    string(REGEX REPLACE "[^a-zA-Z0-9_\\.+-]" "_" SANITISED_FLAG "${FLAG}")
    add_compile_options($<$<BOOL:${SUPPORTS_CMP_${SANITISED_FLAG}}>:${FLAG}>)
endmacro()


macro(check_set_cxx_linker_flag FLAG)
    string(REGEX REPLACE "[^a-zA-Z0-9_-]" "_" SANITISED_FLAG "${FLAG}")
    check_linker_flag(CXX "${FLAG}" "SUPPORTS_LNK_${SANITISED_FLAG}")
endmacro()

macro(check_set_c_linker_flag FLAG)
    string(REGEX REPLACE "[^a-zA-Z0-9_-]" "_" SANITISED_FLAG "${FLAG}")
    check_linker_flag(C "${FLAG}" "SUPPORTS_LNK_${SANITISED_FLAG}")
endmacro()


macro(set_linker_flag_if_supported FLAG)
    string(REGEX REPLACE "[^a-zA-Z0-9_\\.+-]" "_" SANITISED_FLAG "${FLAG}")
    add_link_options($<$<BOOL:${SUPPORTS_LNK_${SANITISED_FLAG}}>:${FLAG}>)
endmacro()

foreach (FLAG IN LISTS CXX_COMPILER_FLAGS)
    check_set_cxx_compiler_flag("${FLAG}")
    set_compile_flag_if_supported("${FLAG}")
endforeach()

foreach (FLAG IN LISTS C_COMPILER_FLAGS)
    check_set_c_compiler_flag("${FLAG}")
    set_compile_flag_if_supported("${FLAG}")
endforeach()

foreach (FLAG IN LISTS CXX_LINKER_FLAGS)
    check_set_cxx_linker_flag("${FLAG}")
    set_linker_flag_if_supported("${FLAG}")
endforeach()

foreach (FLAG IN LISTS C_LINKER_FLAGS)
    check_set_c_linker_flag("${FLAG}")
    set_linker_flag_if_supported("${FLAG}")
endforeach()

check_cxx_source_compiles("
    #include <execution>
    #include <vector>

    int main(void) {
        std::vector<char> v;
        std::sort(std::execution::par, v.begin(), v.end());
    }
" HAS_PAR)

check_cxx_source_compiles("
    #include <execution>
    #include <vector>

    int main(void) {
        std::vector<char> v;
        std::sort(std::execution::par_unseq, v.begin(), v.end());
    }
" HAS_PAR_UNSEQ)

target_compile_definitions("${SANITISED_PROJECT_NAME}" PUBLIC
    $<$<BOOL:${HAS_PAR_UNSEQ}>:HAS_PAR>
    $<$<BOOL:${HAS_PAR_UNSEQ}>:HAS_PAR_UNSEQ>
)

# Configure submodules

message(STATUS "IMAGE_AVIF: ${IMAGE_AVIF}")
message(STATUS "IMAGE_BMP:  ${IMAGE_BMP}")
message(STATUS "IMAGE_GIF:  ${IMAGE_GIF}")
message(STATUS "IMAGE_JPG:  ${IMAGE_JPG}")
message(STATUS "IMAGE_JXL:  ${IMAGE_JXL}")
message(STATUS "IMAGE_LBM:  ${IMAGE_LBM}")
message(STATUS "IMAGE_PCX:  ${IMAGE_PCX}")
message(STATUS "IMAGE_PNG:  ${IMAGE_PNG}")
message(STATUS "IMAGE_PNM:  ${IMAGE_PNM}")
message(STATUS "IMAGE_QOI:  ${IMAGE_QOI}")
message(STATUS "IMAGE_SVG:  ${IMAGE_SVG}")
message(STATUS "IMAGE_TGA:  ${IMAGE_TGA}")
message(STATUS "IMAGE_TIF:  ${IMAGE_TIF}")
message(STATUS "IMAGE_WEBP: ${IMAGE_WEBP}")
message(STATUS "IMAGE_XCF:  ${IMAGE_XCF}")
message(STATUS "IMAGE_XPM:  ${IMAGE_XPM}")
message(STATUS "IMAGE_XV:   ${IMAGE_XV}")

function(set_found LIB FOUND_VAR)
    set(FOUND FALSE)
    foreach(LIBRARY "${LIB}" "lib${LIB}" "${LIB}::${LIB}")
        find_package("${LIBRARY}" QUIET)
        if ("${LIBRARY}_FOUND" OR TARGET "${LIBRARY}")
            set(FOUND TRUE)
            break()
        endif()
    endforeach()

    if (FOUND)
        message("Found library: ${LIB}")
    else()
        message("Could not find library: ${LIB}")
    endif()
    
    set("${FOUND_VAR}" "${FOUND}" PARENT_SCOPE)
endfunction()

function(add_alias_if_valid ALIAS TARGET)
    message(VERBOSE "Setting \"${ALIAS}\" as alias for \"${TARGET}\"")
    
    if (TARGET "${ALIAS}")
        message(VERBOSE "    \"${ALIAS}\" is already a target")
    elseif(NOT TARGET "${ALIAS}")
        message(VERBOSE "    \"${TARGET}\" is not already a target")
    else()
        message(VERBOSE "    \"${ALIAS}\" set as alias for \"${TARGET}\"")
        add_library("${ALIAS}" ALIAS "${TARGET}")
    endif()

    # TODO - add setting variables
endfunction()

CPMAddPackage(
    NAME discord-rpc
    VERSION 1.0.0
    GIT_REPOSITORY https://github.com/EclipseMenu/discord-presence.git
    GIT_TAG 346214d78ace3c4e4e09759f412c3717bbc6e25c
)

CPMAddPackage(
    NAME fmt
    VERSION 11.2.0
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.2.0
)

CPMAddPackage(
    NAME glaze
    VERSION 5.5.4
    GIT_REPOSITORY https://github.com/stephenberry/glaze.git
    GIT_TAG v5.5.4
)

CPMAddPackage(
    NAME cpr
    VERSION 1.12.0
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.12.0
    OPTIONS
        "CPR_USE_SYSTEM_LIB_PSL ON"
        "CURL_USE_LIBPSL OFF"
)

CPMAddPackage(
    NAME SDL3
    VERSION 3.2.20
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.20
    OPTIONS
        "SDL_CCACHE ON"
        "SDL_CLOCK_GETTIME ON"
)

CPMAddPackage(
    NAME SDL3_image
    VERSION 3.2.4
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG release-3.2.4
    OPTIONS
        "SDLIMAGE_VENDORED ON"
        "SDLIMAGE_AVIF ${IMAGE_AVIF}"
        "SDLIMAGE_BMP ${IMAGE_BMP}"
        "SDLIMAGE_GIF ${IMAGE_GIF}"
        "SDLIMAGE_JPG ${IMAGE_JPG}"
        "SDLIMAGE_JXL ${IMAGE_JXL}"
        "SDLIMAGE_LBM ${IMAGE_LBM}"
        "SDLIMAGE_PCX ${IMAGE_PCX}"
        "SDLIMAGE_PNG ${IMAGE_PNG}"
        "SDLIMAGE_PNM ${IMAGE_PNM}"
        "SDLIMAGE_QOI ${IMAGE_QOI}"
        "SDLIMAGE_SVG ${IMAGE_SVG}"
        "SDLIMAGE_TGA ${IMAGE_TGA}"
        "SDLIMAGE_TIF ${IMAGE_TIF}"
        "SDLIMAGE_WEBP ${IMAGE_WEBP}"
        "SDLIMAGE_XCF ${IMAGE_XCF}"
        "SDLIMAGE_XPM ${IMAGE_XPM}"
        "SDLIMAGE_XV ${IMAGE_XV}"
        "CMAKE_POLICY_VERSION_MINIMUM 3.5"
)

CPMAddPackage(
    NAME SDL3_ttf
    VERSION 3.2.2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-3.2.2
    OPTIONS
        "SDLTTF_VENDORED ON"
)

function(link_if_valid TARGET TYPE ITEM)
    if (TARGET "${TARGET}" AND TARGET "${ITEM}")
        target_link_libraries("${TARGET}" "${TYPE}" "${ITEM}")
    endif()
endfunction()

# Add libraries to project

set(LIBRARIES
    discord-rpc

    fmt

    glaze::glaze

    cpr::cpr

    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    SDL3_image::SDL3_image
)

target_link_libraries("${SANITISED_PROJECT_NAME}" ${LIBRARIES})

# Copy resources and binaries after build

if (ANDROID)
    set(LIB_COPY_DIR ${CMAKE_BINARY_DIR}/android/app/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI})
    set(RES_COPY_DIR ${CMAKE_BINARY_DIR}/android/app/src/main/assets)
else()
    set(LIB_COPY_DIR ${CMAKE_BINARY_DIR}/bin)
    set(RES_COPY_DIR ${CMAKE_BINARY_DIR}/bin)
endif()

message(DEBUG "LIB_COPY_DIR: ${LIB_COPY_DIR}")
message(DEBUG "RES_COPY_DIR: ${RES_COPY_DIR}")


file(GLOB_RECURSE DEBUG_BINARIES CONFIGURE_DEPENDS ${CMAKE_BINARY_DIR}/**.pdb)
add_custom_command(
    TARGET "${SANITISED_PROJECT_NAME}"
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf
            "${LIB_COPY_DIR}"
            ${DEBUG_BINARIES}
    COMMENT "Cleaning previous artifacts"
    COMMAND_EXPAND_LISTS
)
add_custom_command(
    TARGET "${SANITISED_PROJECT_NAME}"
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "${LIB_COPY_DIR}"
            "${RES_COPY_DIR}"
    COMMENT "Creating copy directories"
    COMMAND_EXPAND_LISTS
)

add_custom_command(
    TARGET "${SANITISED_PROJECT_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${SANITISED_PROJECT_NAME}>
            $<TARGET_FILE:${SANITISED_PROJECT_NAME}>
            "${LIB_COPY_DIR}"
    COMMENT "Copying runtime artifacts to ${LIB_COPY_DIR}"
    COMMAND_EXPAND_LISTS
)

if (EXISTS ${CMAKE_BINARY_DIR}/${APP_NAME}.pdb)
    add_custom_command(
        TARGET "${SANITISED_PROJECT_NAME}"
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_BINARY_DIR}/${APP_NAME}.pdb
                "${LIB_COPY_DIR}"
        COMMENT "Copying ${APP_NAME}.pdb to ${LIB_COPY_DIR}"
    )
endif()

add_custom_command(
    TARGET "${SANITISED_PROJECT_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            ${CMAKE_SOURCE_DIR}/resources
            ${RES_COPY_DIR}/resources
    COMMENT "Copying resources to ${RES_COPY_DIR}"
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