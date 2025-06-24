# lore-game

[![Build](https://github.com/lore-org/lore-game/actions/workflows/Build.yml/badge.svg)](https://github.com/lore-org/lore-game/actions/workflows/Build.yml) [![wakatime](https://wakatime.com/badge/user/92566484-d54a-42e0-aca3-df99d14ca234/project/cf7c51bb-a878-4877-b0a5-9bb5593da52d.svg)](https://wakatime.com/badge/user/92566484-d54a-42e0-aca3-df99d14ca234/project/cf7c51bb-a878-4877-b0a5-9bb5593da52d)

This is the base repository for the special funky furry game being made. When finished, the engine will be moved to a seperate repository, but for now it is located here.

## Building

To build this project, it is recommended you have CMake installed, as it manages dependencies for you. The Makefile is outdated and should not be used.

This guide also assumes you have a compiler installed such as msvc, clang, or gcc. 

This project requires vcpkg. To install it, run either `./setup/setup-vcpkg.bat` for Windows, or `. setup/setup-vcpkg.bat` for Linux or MacOS.

### VSCode

To build the project in VSCode, make sure you have the recommended extensions installed.

After you install those extensions, press `F1` and run `CMake: Configure` to configure the project.

You may have to select a kit and variant. It is recommended to use `Clang x86_64` as the kit and `Release` or `RelWithDebInfo` for the variant.

After the project is configured, press `F1` again and run `CMake: Build`. Your executable will be located in the build directory along with the required resources.

### Command Line

To configure, run `cmake -D CMAKE_BUILD_TYPE=<build_type> -B build`, replacing `<build_type>` with one `Debug`, `Release`, `MinSizeRel`, or `RelWithDebInfo`. Either `Release` or `RelWithDebInfo` are recommended.

After configuring, run `cmake --build build` to build the project. Your executable will be located in the build directory along with the required resources.

### GitHub Actions

This repo also provides commands for cross-platform building when a commit is pushed. Visit the [actions](actions/workflows/Build.yml) tab to view recent builds, and download the proper artifacts for your system.

## Debugging

The described debugging method requires VSCode. Visual Studio and some other IDEs have a built-in debugging method.

Currently, the base debug system is broken and does not parse symbols correctly. Instead, press `F1` and run `CMake: Debug`, or press `Shift + F5` if the CMake debug hotkey is registered.

## Troubleshooting

```
Could NOT find X11 (missing: X11_X11_INCLUDE_PATH X11_X11_LIB)
```

Install X11 developer tools with `sudo apt-get install xorg-dev`. You may need to update your repositories with `sudo apt-get update`