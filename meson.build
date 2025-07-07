project(
    'lore-game',
    'cpp',
    version: '0.0.1',
    default_options: [
        'default_library=shared',
        'warnlevel=everything',
    ],
)

python = find_program('py', 'python3', 'python', required: true)

fmt_dep = dependency('fmt')
glaze_dep = dependency('glaze')
sdl3_dep = dependency('sdl3')
sdl3_ttf_dep = dependency('sdl3_ttf')
sdl3_image_dep = dependency('sdl3_image')
cpr_dep = dependency('cpr')

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
    dependencies: [fmt_dep, glaze_dep],
    cpp_args: '-std=c++23',
)

config_file = run_command(python, files('generate-config.py'), check: false).stdout().strip()

sources = files('src/main.cpp')
subdir('src/engine')
executable(
    'lore-game',
    sources,
    dependencies: [
        sdl3_dep,
        sdl3_ttf_dep,
        sdl3_image_dep,
        fmt_dep,
        glaze_dep,
        cpr_dep,
    ],
    link_with: discord_presence,
    include_directories: include_directories('include', discord_presence_include),
    cpp_args: '-std=c++23',
)
