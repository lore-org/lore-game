#pragma once
#include "Default.hpp" // IWYU pragma: keep

#include "Object.hpp"

class Engine;
namespace utils { class PresenceManager; }

namespace {
    inline utils::PresenceManager* g_presenceManager;
}

namespace utils {
    class PresenceManager : public Object {
    public:
        static PresenceManager* sharedManager() {
            if (!g_presenceManager) g_presenceManager =  new PresenceManager();
            return g_presenceManager;
        }

        // default is false
        inline void enableRPC(bool enable) {
            m_rpcIsEnabled = enable;
        }

        inline bool isEnabled() {
            return m_rpcIsEnabled;
        }

        inline bool isActive() {
            return m_rpcIsActive;
        }

    protected:
        PresenceManager() : m_rpcIsEnabled(false), m_rpcIsActive(false) {};
    
        bool m_rpcIsEnabled;
        bool m_rpcIsActive;

        inline void setActive(bool active) {
            m_rpcIsActive = active;
        }

        friend class ::Engine;
    };
}