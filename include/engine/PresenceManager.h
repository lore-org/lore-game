#pragma once

#include <memory>

#include "Object.h"
#include "Engine.h"

namespace utils {
    class PresenceManager : public Object {
        friend class ::std::shared_ptr<PresenceManager>;
        friend class ::Engine;

    public:

        static std::shared_ptr<PresenceManager> sharedManager();

        // Default is false
        void enableRPC(bool enable);

        bool isEnabled();
        bool isActive();

    protected:
        PresenceManager();
    
        bool m_rpcIsEnabled;
        bool m_rpcIsActive;

        void setActive(bool active);
    
    private:
        static std::shared_ptr<PresenceManager> m_instance;
    };
}