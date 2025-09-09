# lore-game

[![Build](https://github.com/lore-org/lore-game/actions/workflows/Build.yml/badge.svg)](https://github.com/lore-org/lore-game/actions/workflows/Build.yml) [![wakatime](https://wakatime.com/badge/user/92566484-d54a-42e0-aca3-df99d14ca234/project/cf7c51bb-a878-4877-b0a5-9bb5593da52d.svg)](https://wakatime.com/badge/user/92566484-d54a-42e0-aca3-df99d14ca234/project/cf7c51bb-a878-4877-b0a5-9bb5593da52d)

This is the base repository for the special funky furry game being made. When finished, the engine will be moved to a seperate repository, but for now it is located here.

## Building

To build this project, you need to have [cmake](https://cmake.org/download/) >=v3.24.0 installed.

This guide also assumes you have a c and c++ compiler installed that supports most of C++23, such as [Clang](https://github.com/llvm/llvm-project/releases/latest) or [GCC](https:/gcc.gnu.org/install/binaries.html).

It is reccomended you use Clang >=v20.0.0 for building this project.

For Windows, it is reccomended you install [scoop](https://scoop.sh/) to manage software.

### VSCode

To build the project in VSCode, make sure you have the recommended extensions installed.

After you install those extensions, press `F1` and run `CMake: Build` to configure and build the project. Your executable will be located in `build/bin` along with the required resources and libraries.

If you need to change the projects build options, you can edit the configurable definitions located in the [CMakeLists.txt](CMakeLists.txt).

### Command Line

To configure, run `cmake -B build -S .` in the root directory.

To build, run `cmake --build` to build the project. You can also add `-j $(nproc)` to force cmake to use as many functional processors as your host machine will allow. Your executable will be located in `build/bin` along with the required resources and libraries.

If you need to change the projects build options, you can edit the configurable definitions located in the [CMakeLists.txt](CMakeLists.txt), or add options via optional command-line arguments, such as `-D BUILD_SHARED_LIBS=ON`.

### GitHub Actions

This repo also provides commands for cross-platform building when a commit is pushed. Visit the [actions](https://github.com/lore-org/lore-game/actions/workflows/Build.yml) tab to view recent builds, and download the proper artifacts for your system.

## Debugging

The described debugging method requires VSCode. Visual Studio and some other IDEs have a built-in debugging method.

To debug the program, press `F1` and run `View: Show Run and Debug`, and then hit Start Debugging with target `Debug target`, or press `F5` if the CMake debug hotkey is registered.

You will need to build the project before debugging if you want changes to properly be reflected, or uncomment `"preLaunchTask": "Build",` in [launch.json](.vscode/launch.json).

## Adding Dependencies

When adding a dependency, first make sure it has CMake support, or there is another repository which adds cmake support. If it does not, please follow the guide for [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake#readme)

It is recommended to add `CPM_SOURCE_CACHE` to the systems PATH in order to cache dependency sources.

## Troubleshooting

¯\\\_(ツ)_/¯
