project(
    'lore-game',
    'cpp',
    version: '0.0.1',
    default_options: ['cpp_std=c++23', 'default_library=shared', 'warnlevel=everything'],
)

fmt_dep = dependency('fmt')

discord_presence_sources = [
    'submodules/discord-presence/src/discord-rpc.cpp',
    'submodules/discord-presence/src/serialization.cpp',
]

if host_machine.system() == 'windows'
    discord_presence_sources += 'submodules/discord-presence/src/platform/windows.cpp'
endif

discord_presence_include = 'submodules/discord-presence/include'

discord_presence = library(
    'discord-presence',
    discord_presence_sources,
    include_directories: include_directories(discord_presence_include),
    dependencies: [fmt_dep, dependency('glaze')],
    cpp_args: '-std=c++23',
)

sources = files('src/main.cpp')

subdir('src/engine')

executable(
    'lore-game',
    sources,
    dependencies: [
        dependency('sdl3'),
        dependency('sdl3_ttf'),
        dependency('sdl3_image'),
        fmt_dep,
        dependency('nlohmann_json'),
        dependency('cpr'),
    ],
    link_with: discord_presence,
    include_directories: include_directories('include', discord_presence_include),
    cpp_args: '-std=c++23',
)
