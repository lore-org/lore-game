with open('config.json', 'r') as f:
    config = f.read().strip()

config_header = f'''\
#pragma once
#include <glaze/glaze.hpp>

inline glz::json_t config = 
glz::read_json(config, static_cast<std::string>(R"({config})"));\
'''

with open('include/engine/config.hpp', 'w') as f:
    f.write(config_header)