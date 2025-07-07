with open('config.json', 'r') as f:
    config = f.read().strip()

with open('include/engine/config.hpp', 'w') as f:
    f.write(
f'''\
#pragma once

#include <glaze/glaze.hpp>

inline auto config = glz::read_json<glz::json_t>(R"({config})");\
'''
    )