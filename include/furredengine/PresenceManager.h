#pragma once

#include <memory>
#include <string>

#include <furredengine/Object.h>

namespace FurredEngine {

class PresenceManager : public Object {
    friend class Engine;

public:

    static std::shared_ptr<PresenceManager> sharedManager();

    void startRPC(std::string clientID);
    
    inline void restartOnFail(bool which) { m_restartOnFail = which; }
    inline bool restartOnFail() { return m_restartOnFail; }
    inline bool isActive() { return m_isActive; }

protected:
    PresenceManager();

private:
    static std::shared_ptr<PresenceManager> m_instance;

    bool m_isActive;
    bool m_restartOnFail;
};

}