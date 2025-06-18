#pragma once
#include "Default.hpp"

inline bool rpcIsEnabled = false;
inline bool rpcIsActive = false;

namespace presence {
    // default is false
    inline void enableRPC(bool enable) {
        rpcIsEnabled = enable;
    }

    inline bool isEnabled() {
        return rpcIsEnabled;
    }

    inline bool isActive() {
        return rpcIsActive;
    }
}