include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
include(CheckLinkerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFile)

cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ----- Options -----

set(CMAKE_ANDROID_ARCH_ABI arm64-v8a CACHE STRING "What architecture to compile the Android application for")

set(CMAKE_ANDROID_API 33 CACHE STRING "Target Android API version. Default is Android 13.")
set(CMAKE_ANDROID_API_MIN 24 CACHE STRING "Minimum Android API version. Default is Android 7.0.")

set(BUILD_SHARED_LIBS OFF CACHE STRING "Build .dlls, .so, and .dylibs.")

project(
    "Lore Game"
    LANGUAGES C CXX
    VERSION 0.0.1
)

set(APP_NAMESPACE "com.furred.${PROJECT_NAME}" CACHE STRING "The namespace given to the application.")
set(APP_NAME "${PROJECT_NAME}" CACHE STRING "The name given to the application.")

set(ANDROID_SPLASH_BACKGROUND_COLOR "#FFFFFF" CACHE STRING "The background colour for the Android splash screen.")

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

execute_process(COMMAND ${PYTHON} -m ensurepip --upgrade)
execute_process(COMMAND ${PYTHON} -m pip install -r ${CMAKE_SOURCE_DIR}/python/requirements.txt)

# Env detection

if (NOT DEFINED ANDROID_SDK OR NOT DEFINED ANDROID_HOME)
    if (ANDROID AND NOT DEFINED ENV{ANDROID_SDK} AND NOT DEFINED ENV{ANDROID_HOME}) # don't error on non-android build platforms
        message(FATAL_ERROR "ANDROID_SDK or ANDROID_HOME must be defined as a cmake variable or environment variable.")
    endif()

    if (DEFINED ENV{ANDROID_SDK})
        set(ANDROID_SDK "$ENV{ANDROID_SDK}")
    endif()
    if (DEFINED ENV{ANDROID_HOME})
        set(ANDROID_HOME "$ENV{ANDROID_HOME}")
    endif()

    if (NOT DEFINED ANDROID_SDK AND DEFINED ANDROID_HOME)
        set(ANDROID_SDK "${ANDROID_HOME}")
    elseif (NOT DEFINED ANDROID_HOME AND DEFINED ANDROID_SDK)
        set(ANDROID_HOME "${ANDROID_SDK}")
    endif()

    file(TO_CMAKE_PATH "${ANDROID_SDK}" ANDROID_SDK)
    set(ANDROID_SDK "${ANDROID_SDK}" CACHE STRING "")

    file(TO_CMAKE_PATH "${ANDROID_HOME}" ANDROID_HOME)
    set(ANDROID_HOME "${ANDROID_HOME}" CACHE STRING "")
endif()

if (NOT DEFINED ANDROID_NDK)
    if (ANDROID AND NOT DEFINED ENV{ANDROID_NDK}) # don't error on non-android build platforms
        message(FATAL_ERROR "ANDROID_NDK must be defined as a cmake variable or environment variable.")
    endif()
    file(TO_CMAKE_PATH "$ENV{ANDROID_NDK}" ANDROID_NDK)
    set(ANDROID_NDK "${ANDROID_NDK}" CACHE STRING "") # libwebp relies on a cmake ANDROID_NDK instead of an env ANDROID_NDK
endif()

if (NOT DEFINED JAVA_HOME)
    if (ANDROID AND NOT DEFINED ENV{JAVA_HOME}) # don't error on non-android build platforms
        message(FATAL_ERROR "JAVA_HOME must be defined as a cmake variable or environment variable.")
    endif()
    file(TO_CMAKE_PATH "$ENV{JAVA_HOME}" JAVA_HOME)
    set(JAVA_HOME "${JAVA_HOME}" CACHE STRING "")
endif()

file(TO_CMAKE_PATH ${HOME_DIR}/.android/${SANITISED_PROJECT_NAME}_debug.keystore ANDROID_DEBUG_KEYSTORE)
if (ANDROID AND NOT EXISTS ${ANDROID_DEBUG_KEYSTORE})
    find_program(KEYTOOL keytool REQUIRED)

    string(TIMESTAMP CURRENT_UNIX_TIME "%s")
    set(MAXIMUM_UNIX_TIME "253402232400")

    math(EXPR MAX_VALIDITY_DAYS "((${MAXIMUM_UNIX_TIME}+0) - (${CURRENT_UNIX_TIME}+0)) / 60 / 60 / 24")

    execute_process(
        COMMAND ${KEYTOOL}
        -genkey
        -keystore ${ANDROID_DEBUG_KEYSTORE}
        -storepass android
        -keypass android
        -alias AndroidDebugKey
        -keyalg RSA
        -validity ${MAX_VALIDITY_DAYS}
        -dname "CN=Unknown,OU=Unknown,O=Unknown,L=Unknown,ST=Unknown,C=Unknown"
    )
endif()

# Init project

if (ANDROID)
    add_library("${SANITISED_PROJECT_NAME}" SHARED)
else()
    add_executable("${SANITISED_PROJECT_NAME}")
    set_target_properties("${SANITISED_PROJECT_NAME}" PROPERTIES OUTPUT_NAME "${APP_NAME}")
endif()

set_target_properties("${SANITISED_PROJECT_NAME}" PROPERTIES CXX_STANDARD 23)

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

# Allow on RelWithDebInfo
if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
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

execute_process(COMMAND ${PYTHON} ${CMAKE_SOURCE_DIR}/python/icon_generator.py --crop-type longest --build-dir ${CMAKE_BINARY_DIR} --resources-dir ${CMAKE_SOURCE_DIR}/resources)

# Linker/Compiler checking

list(APPEND COMPILER_FLAGS
    "-Wno-c++98-compat"
    "-Wno-error"
    "-Wno-unused-command-line-argument"
    "-Wno-missing-template-arg-list-after-template-kw"

    "-Wpedantic"

    "-fPIC"
    "-fexperimental-library"

    "-std=c++23" # prevent use of gnu++23
    # "-stdlib=libc++"
    # "-static-libstdc++"
)

if (DEBUG)
    list(APPEND COMPILER_FLAGS
        "-gcodeview"
        "-gcolumn-info"

        "-fuse-ld=lld"
        "-fno-omit-frame-pointer"
    )
endif()


foreach (FLAG IN LISTS COMPILER_FLAGS)
    string(REGEX REPLACE "[^a-zA-Z0-9_-]" "_" SANITISED_FLAG "${FLAG}")


    check_cxx_compiler_flag("${FLAG}" CXX_SUPPORTS_CMP_${SANITISED_FLAG})
    if (CXX_SUPPORTS_CMP_${SANITISED_FLAG})
        check_c_compiler_flag("${FLAG}" C_SUPPORTS_CMP_${SANITISED_FLAG})
        if (C_SUPPORTS_CMP_${SANITISED_FLAG})
            add_compile_options(${FLAG})
        endif()
    endif()

    check_linker_flag(CXX "${FLAG}" CXX_SUPPORTS_LNK_${SANITISED_FLAG})
    if (CXX_SUPPORTS_LNK_${SANITISED_FLAG})
        check_linker_flag(C "${FLAG}" C_SUPPORTS_LNK_${SANITISED_FLAG})
        if (C_SUPPORTS_LNK_${SANITISED_FLAG})
            add_link_options(${FLAG})
        endif()
    endif()
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
    $<$<BOOL:${HAS_PAR}>:HAS_PAR>
    $<$<BOOL:${HAS_PAR_UNSEQ}>:HAS_PAR_UNSEQ>
)

# Configure submodules

find_package(Git REQUIRED)
message("Git_VERSION:           ${Git_VERSION}")

CPMAddPackage(
    NAME discord-rpc
    GIT_REPOSITORY https://github.com/EclipseMenu/discord-presence.git
    GIT_TAG 59a3ba7
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
    NAME simdutf
    VERSION 7.7.0
    GIT_REPOSITORY https://github.com/simdutf/simdutf.git
    GIT_TAG v7.7.0
)

if (ANDROID)
    set(OpenSSL_CROSS "CROSS ON")
    set(OpenSSL_CROSS_TARGET "CROSS_TARGET mingw")
    set(OpenSSL_CROSS_ANDROID "CROSS_ANDROID ON")
endif()
if (APPLE)
    set(OpenSSL_BUILD_OPENSSL "BUILD_OPENSSL ON")
endif()
CPMAddPackage(
    NAME OpenSSL
    VERSION 3.6.0
    GIT_REPOSITORY https://github.com/viaduck/openssl-cmake.git
    GIT_TAG 7e0c2c4
    OPTIONS
        "OPENSSL_BUILD_VERSION 3.6.0"
        "${OpenSSL_CROSS}"
        "${OpenSSL_CROSS_TARGET}"
        "${OpenSSL_CROSS_ANDROID}"
        "${OpenSSL_BUILD_OPENSSL}"
)
if(NOT EXISTS OpenSSL::SSL)
    add_library(OpenSSL::SSL ALIAS ssl)
endif()
if(NOT EXISTS OpenSSL::Crypto)
    add_library(OpenSSL::Crypto ALIAS crypto)
endif()

CPMAddPackage(
    NAME Ada
    VERSION 3.4.2
    GIT_REPOSITORY https://github.com/ada-url/ada.git
    GIT_TAG v3.4.2
    OPTIONS
        "ADA_USE_SIMDUTF ON"
)
target_include_directories(
    "${SANITISED_PROJECT_NAME}"
    PUBLIC ${Ada_SOURCE_DIR}/include
    PUBLIC ${Ada_SOURCE_DIR}/src
)

CPMAddPackage(
    NAME cpp-httplib
    VERSION 0.33.1
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.33.1
    OPTIONS
        "HTTPLIB_REQUIRE_OPENSSL ON"
)

CPMAddPackage(
    NAME GLFW
    VERSION 3.4
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    OPTIONS
        "GLFW_BUILD_DOCS OFF"
)

CPMAddPackage(
    NAME glm
    VERSION 1.0.2
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.2
)

CPMAddPackage(
    NAME harfbuzz
    VERSION 12.3.2
    GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
    GIT_TAG 12.3.2
)

CPMAddPackage(
    NAME FreeType
    VERSION 2.14.1
    GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
    GIT_TAG VER-2-14-1
    OPTIONS
        "FT_ENABLE_ERROR_STRINGS ON"
)

CPMAddPackage(
    NAME rectpack2D
    GIT_REPOSITORY https://github.com/TeamHypersomnia/rectpack2D.git
    GIT_TAG bbc9d0e
)

# Install Glad module for Python

execute_process(COMMAND ${PYTHON} -m ensurepip --upgrade)
execute_process(COMMAND ${PYTHON} -m pip install -r ${CMAKE_SOURCE_DIR}/python/glad_requirements.txt)

# Set up glad

CPMAddPackage(
    NAME glad
    VERSION 2.0.8
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.8
)
include(${glad_SOURCE_DIR}/cmake/CMakeLists.txt)

check_include_file("X11/X.h" HAS_X11)
if (HAS_X11)
    set(GLAD_GLX "glx=1.4")
else()
    set(GLAD_GLX "")
endif()

if (ANDROID)
    set(GLAD_GLES1 "gles1:common=1.0")
    set(GLAD_GLES2 "gles2=3.2")
else()
    set(GLAD_GL "gl:core=3.2")
endif()

glad_add_library(
    glad_gl_3_2
    API
        "${GLAD_GL}"
        "${GLAD_GLES1}"
        "${GLAD_GLES2}"
        "${GLAD_GLX}"
)

if (HAS_X11)
    target_compile_options(glad_gl_3_2 PUBLIC -lX11)
endif()

# Set up stb

CPMAddPackage(
    NAME stb
    GIT_REPOSITORY https://github.com/nothings/stb
    GIT_TAG f1c79c0
    DOWNLOAD_ONLY
)

target_include_directories("${SANITISED_PROJECT_NAME}" PUBLIC ${stb_SOURCE_DIR})

# Add libraries to project

target_link_libraries("${SANITISED_PROJECT_NAME}"
    discord-rpc
    fmt
    glaze::glaze
    ssl
    crypto
    ada
    httplib
    glfw
    simdutf
    glad_gl_3_2
    glm
    harfbuzz
    freetype
    rectpack2D
)

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

if (WIN32)
    # TARGET_RUNTIME_DLLS
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
else()
    # GLOB_RECURSE
    file(GLOB_RECURSE SHARED_LIBS CONFIGURE_DEPENDS ${CMAKE_BINARY_DIR}/_deps/*-build/**${CMAKE_SHARED_LIBRARY_SUFFIX})

    add_custom_command(
        TARGET "${SANITISED_PROJECT_NAME}"
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${SHARED_LIBS}
                $<TARGET_FILE:${SANITISED_PROJECT_NAME}>
                "${LIB_COPY_DIR}"
        COMMENT "Copying runtime artifacts to ${LIB_COPY_DIR}"
        COMMAND_EXPAND_LISTS
    )
endif()

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
    )
endif()