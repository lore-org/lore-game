#pragma once

#include <nlohmann/json.hpp>

inline const char configString[] = {
    #embed "../config.json"
};

inline const nlohmann::json config = nlohmann::json::parse(configString);