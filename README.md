# lore-game

[![Build](https://github.com/lore-org/lore-game/actions/workflows/Build.yml/badge.svg)](https://github.com/lore-org/lore-game/actions/workflows/Build.yml) [![wakatime](https://wakatime.com/badge/user/92566484-d54a-42e0-aca3-df99d14ca234/project/cf7c51bb-a878-4877-b0a5-9bb5593da52d.svg)](https://wakatime.com/badge/user/92566484-d54a-42e0-aca3-df99d14ca234/project/cf7c51bb-a878-4877-b0a5-9bb5593da52d)

This is the base repository for the special funky furry game being made. When finished, the engine will be moved to a seperate repository, but for now it is located here.

## Building

To build this project, you need to have [xmake](https://xmake.io/) installed.

This guide also assumes you have a compiler installed such as msvc, clang, or gcc.

### VSCode

To build the project in VSCode, make sure you have the recommended extensions installed.

After you install those extensions, press `F1` and run `XMake: Build` to build the project. Your executable will be located in the build directory along with the required resources and libraries.

### Command Line

To build, run `xmake` to build the project. Your executable will be located in the build directory along with the required resources.

### GitHub Actions

This repo also provides commands for cross-platform building when a commit is pushed. Visit the [actions](https://github.com/lore-org/lore-game/actions/workflows/Build.yml) tab to view recent builds, and download the proper artifacts for your system.

## Debugging

The described debugging method requires VSCode. Visual Studio and some other IDEs have a built-in debugging method.

To debug the program, press `F1` and run `XMake: Debug`, or press `F5` if the xmake debug hotkey is registered.

## Adding Dependencies

When adding a dependency, first look for it on [xrepo](https://xrepo.xmake.io/). If you can't find it there, you can use another source

### xrepo Dependencies

To find your package, search for it on the command line with `xrepo search <dependency>`, where `<dependency>` is the name of the package you want to install.

If no results are found, skip to the next section.

Next, copy the name of the dependency you want from the list (e.g. `pugixml` from `  -> pugixml-v1.15: Light-weight, simple and fast XML parser for C++ with XPath support (in xmake-repo)`), then add that dependency to [xmake.lua](xmake.lua) with `add_requires("<dependency>")` (e.g. `add_requires("pugixml")`)

After declaring the dependency, add it to the list in `add_packages` under 
`target("<project>")`, project in this case being `lore-game`.

When finished, your [xmake.lua](xmake.lua) should look something like this:

```lua
set_languages("cxx20")

add_requireconfs("*", {configs = {shared = true}})
add_requires("raylib")
add_requires("fmt")
add_requires("pugixml") -- our package

target("lore-game")
    set_kind("binary")
    add_files("src/*.cpp")
    add_packages("raylib", "fmt", "pugixml") -- added to `add_packages`
```

### Non-xrepo but Remote Dependencies

Some dependencies may not be available through xrepo, but are available elsewhere. to use these, prefix the library with the source's name, e.g. `vcpkg::easyexif`, `brew::pcre2`.

### Git Repository

If the dependency is not available on a package manager, but the dependency does have a git repository (github, gitlab, etc.), you can add it with xmake's [`package`](https://xmake.io/guide/package-management/using-local-packages.html)

For detailed information, please refer to the [guide](https://xmake.io/guide/package-management/using-local-packages.html).

---

In addition to [`package`](https://xmake.io/guide/package-management/using-local-packages.html), you can also define a library from an existing source.

To make sure the source is loaded, add it to your git submodules with `git submodule add <git repo>`.

Here's an example of how to use an external library, using [EclipseMenu/discord-presence](https://github.com/EclipseMenu/discord-presence.git):

```lua
target("discord-presence")
    set_kind("shared") -- library can be shared or static. Defaults to static.

    on_load(function ()
        os.exec("git submodule update --init --recursive")
    end) -- updates the submodules to ensure the repository is downloaded.

    add_rules("utils.symbols.export_all", {export_classes = true}) -- required for windows libraries that don't use __dllspec, which is most.
    add_files("discord-presence/src/**.cpp", "discord-presence/src/**.c") -- compiles all .c and .cpp files in all directories inside src, however most projects only require one cpp file to be built.
    add_includedirs("discord-presence/include") -- allows for global access to included files, like `#include <discord-rpc.hpp>`.

    add_packages("glaze", "fmt") -- these are added with `add_requires`, and are not defined in xmake.lua.
    set_languages("cxx23") -- c++23 is required to build this library.
```

---

After defining a library, it must be linked to your binary to function properly.

To do this, use `add_deps`, NOT `add_packages`.

```lua
target("lore-game")
    set_kind("binary")

    add_files("src/*.cpp")

    add_deps("matjson", "discord-rpc")
```

## Troubleshooting

¯\\\_(ツ)_/¯
