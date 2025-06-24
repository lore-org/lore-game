# lore-game

[![Build](https://github.com/lore-org/lore-game/actions/workflows/Build.yml/badge.svg)](https://github.com/lore-org/lore-game/actions/workflows/Build.yml) [![wakatime](https://wakatime.com/badge/user/92566484-d54a-42e0-aca3-df99d14ca234/project/cf7c51bb-a878-4877-b0a5-9bb5593da52d.svg)](https://wakatime.com/badge/user/92566484-d54a-42e0-aca3-df99d14ca234/project/cf7c51bb-a878-4877-b0a5-9bb5593da52d)

This is the base repository for the special funky furry game being made. When finished, the engine will be moved to a seperate repository, but for now it is located here.

## Building

To build this project, it is recommended you have CMake installed, as it manages dependencies for you. The Makefile is outdated and should not be used.

This guide also assumes you have a compiler installed such as msvc, clang, or gcc.

### VSCode

To build the project in VSCode, make sure you have the recommended extensions installed.

After you install those extensions, press `F1` and run `CMake: Configure` to configure the project.

You may have to select a kit and variant. It is recommended to use `Clang x86_64` as the kit and `Release` or `RelWithDebInfo` for the variant.

After the project is configured, press `F1` again and run `CMake: Build`. Your executable will be located in the build directory along with the required resources.

### Command Line

To configure, run `cmake -D CMAKE_BUILD_TYPE=<build_type> -B build`, replacing `<build_type>` with one `Debug`, `Release`, `MinSizeRel`, or `RelWithDebInfo` (e.g. `cmake -D CMAKE_BUILD_TYPE=RelWithDebInfo -B build`).  Either `Release` or `RelWithDebInfo` are recommended.

After configuring, run `cmake --build build` to build the project. Your executable will be located in the build directory along with the required resources.

### GitHub Actions

This repo also provides commands for cross-platform building when a commit is pushed. Visit the [actions](https://github.com/lore-org/lore-game/actions/workflows/Build.yml) tab to view recent builds, and download the proper artifacts for your system.

## Debugging

The described debugging method requires VSCode. Visual Studio and some other IDEs have a built-in debugging method.

Currently, the base debug system is broken and does not parse symbols correctly. Instead, press `F1` and run `CMake: Debug`, or press `Shift + F5` if the CMake debug hotkey is registered.

## Adding Dependencies

When adding a dependency, first look for it on [vcpkg.io/packages](https://vcpkg.io/en/packages). If you can't find it there, a git repository that can be built with CMake works fine.

### vcpkg Dependencies

If you have vcpkg installed to your system path, run `vcpkg add port <dependency>`, replacing `<dependency>` with the name of the dependency you want to install with the project (e.g. [`vcpkg add port pugixml`](https://vcpkg.io/en/package/pugixml)).

If you don't have vcpkg installed to your system path, instead add the dependency name to [vcpkg.json](vcpkg.json), as shown below.

```jsonc
{
  "dependencies": [
    "curl",
    "nlohmann-json",
    // more dependencies...
    "pugixml"
  ]
}
```

> [!NOTE]  
> If you have configured CMake once already, vcpkg will have been bootstrapped, and you can run it with `./vcpkg/vcpkg.exe` or `./vcpkg/vcpkg`, respective of your system. For ease of access, you can add `/vcpkg` to your path. [Read here for more info](https://gist.github.com/nex3/c395b2f8fd4b02068be37c961301caa7).

After adding the dependency to [vcpkg.json](vcpkg.json), you have to link it to the binary in [CMakeLists.txt](CMakeLists.txt).

To do so, open [CMakeLists.txt](CMakeLists.txt) and scroll down until you see `# Adding vcpkg packages`, and add the dependency to the list of `find_package`s, as shown below.

```cmake
# Adding vcpkg packages

find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
# more dependencies...
find_package(pugixml REQUIRED)
```

Then, add add the dependency name to `target_link_libraries`.

```cmake
# Add static libraries to project

target_link_libraries(${PROJECT_NAME} PUBLIC
    CURL::libcurl
    nlohmann_json::nlohmann_json
    # more dependencies...
    pugixml
)
```

When finished, your [CMakeLists.txt](CMakeLists.txt) should look like this, granted you're installing the [`pugixml`](https://vcpkg.io/en/package/pugixml) dependency:

```cmake
# Adding vcpkg packages

find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)

# Add static libraries to project

target_link_libraries(${PROJECT_NAME} PUBLIC
    CURL::libcurl
    nlohmann_json::nlohmann_json
    pugixml
)
```

> [!WARNING]  
> If you run into an error after adding a package, try reading back the configuration logs. vcpkg will print a short guide on how to add each library to your project, such as `pugixml provides CMake targets:`. These may not always be correct, however, so it's important to look online for solutions.

### Git Repository

If the dependency is not available in the vcpkg package directory, but the dependency does have a git repository (github, gitlab, etc.), you can add it with CMake's [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html)

To do so, open [CMakeLists.txt](CMakeLists.txt) and scroll down until you see `# Adding non-vcpkg packages`, and add the dependency to the list of `FetchContent_Declare`s, as shown below.

```cmake
# Adding non-vcpkg packages

include(FetchContent)

FetchContent_Declare(
    discord-rpc
    GIT_REPOSITORY https://github.com/EclipseMenu/discord-presence.git
    GIT_TAG main
)
# more dependencies...
FetchContent_Declare(
    spotify-api-plusplus
    GIT_REPOSITORY https://github.com/smaltby/spotify-api-plusplus.git
    GIT_TAG master # will default to master, but can still be set for clarity
)
```

Then, add add the dependency name to `FetchContent_MakeAvailable`.

```cmake
FetchContent_MakeAvailable(
    discord-rpc
    # more dependencies...
    spotify-api-plusplus
)
```

After this, the dependency must now be added to `target_link_libraries` to be included in the binary.

```cmake
# Add static libraries to project

target_link_libraries(${PROJECT_NAME} PUBLIC
    discord-rpc
    # more dependencies...
    spotify-api-plusplus

    CURL::libcurl
    nlohmann_json::nlohmann_json
    pugixml
)
```

When finished, your [CMakeLists.txt](CMakeLists.txt) should look like this, granted you're installing the [`spotify-api-plusplus`](https://github.com/smaltby/spotify-api-plusplus) dependency:

```cmake
# Adding non-vcpkg packages

include(FetchContent)

FetchContent_Declare(
    discord-rpc
    GIT_REPOSITORY https://github.com/EclipseMenu/discord-presence.git
    GIT_TAG main
)
FetchContent_Declare(
    spotify-api-plusplus
    GIT_REPOSITORY https://github.com/smaltby/spotify-api-plusplus.git
    GIT_TAG master # will default to master, but can still be set for clarity
)

FetchContent_MakeAvailable(
    discord-rpc
    spotify-api-plusplus
)

# Adding vcpkg packages...

# Add static libraries to project

target_link_libraries(${PROJECT_NAME} PUBLIC
    discord-rpc
    spotify-api-plusplus

    CURL::libcurl
    nlohmann_json::nlohmann_json
    pugixml
)
```

> [!WARNING]  
> Not all repositories will work with FetchContent. If you run into an error when configuring, it is likely that the library 's CMakeLists.txt does not contain `add_library`. Please continue reading for a solution.

### Non-CMake, Non-vcpkg Library

If neither of the above guides work for you, please try the guide at [learn.microsoft.com/vcpkg](https://learn.microsoft.com/vcpkg/examples/packaging-github-repos), or add custom build steps in your [CMakeLists.txt](CMakeLists.txt) to compile the library. If needed, please create an issue describing your problem.

## Troubleshooting

```
Could NOT find X11 (missing: X11_X11_INCLUDE_PATH X11_X11_LIB)
```

Install X11 developer tools with `sudo apt-get install xorg-dev`. You may need to update your repositories with `sudo apt-get update`
