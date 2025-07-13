import shutil
import glob
import os
import argparse
import re

parser = argparse.ArgumentParser(description='Installs all libraries, executables, and resources to /build/bin')

parser.add_argument("--search-depth", help="How far to search for dynamic libraries", default=3, type=int)
parser.add_argument("--project-name", help="Specify a name for the CMake project", type=str)

args = parser.parse_args()

if args.project_name:
    project_name = args.project_name
    print(f"Using specified project name: '{project_name}'")
else:
    with open("CMakeLists.txt", "r") as file:
        cmake = file.read()

    cmake_match = re.search(r"project\([\s\n]*\"?([\w_-]+)", cmake)
    assert cmake_match, "Could not parse CMakeLists for project name. Please supply it through '--project-name'"
    project_name = cmake_match.group(1)
    print(f"Found project name: '{project_name}'")

binary_types = ('.exe', '.pdb', '')
library_types = ('.dll', '.so', '.dylib')
library_prefixes = ('lib', '')

copy_queue = []
bad_dirs = [
    os.path.join('build', 'bin')
]

for i in range(args.search_depth + 1):
    path = ['*' for _ in range(i)]

    # binaries

    for binary_type in binary_types:
        found_paths = glob.glob(os.path.join('build', *path, f'{project_name}{binary_type}'))
        for found_path in found_paths:
            for dir in bad_dirs:
                if found_path.startswith(dir): continue
            
                print(f"Found binary '{os.path.basename(found_path)}'")
                copy_queue += [found_path]


    # submodules

    for library_type in library_types:
        found_paths = glob.glob(os.path.join('build', 'submodules', *path, f'*{library_type}'))
        for found_path in found_paths:
            for dir in bad_dirs:
                if found_path.startswith(dir): continue

                print(f"Found submodule '{os.path.basename(found_path)}'")
                copy_queue += [found_path]


    # _deps

    for library_type in library_types:
        found_paths = glob.glob(os.path.join('build', '_deps', *path, f'*{library_type}'))
        for found_path in found_paths:
            for dir in bad_dirs:
                if found_path.startswith(dir): continue

                print(f"Found external dependency '{os.path.basename(found_path)}'")
                copy_queue += [found_path]


# copy

try:
    os.mkdir(os.path.join('build', 'bin'))
except:
    print

for resource in copy_queue:
    try:
        shutil.copy(resource, os.path.join('build', 'bin'))
    except:
        continue

shutil.copytree('resources', os.path.join('build', 'bin', 'resources'), dirs_exist_ok=True)