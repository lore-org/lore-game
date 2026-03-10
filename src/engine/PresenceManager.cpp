#include <furredengine/PresenceManager.h>

#include <memory>

#include <discord-rpc.hpp>

#include <furredengine/config.hpp>
#include <furredengine/Engine.h>
#include <furredengine/Geometry.h>
#include <furredengine/utils.h>

using namespace FurredEngine;

std::shared_ptr<PresenceManager> PresenceManager::m_instance;

PresenceManager::PresenceManager() : m_rpcIsEnabled(false), m_rpcIsActive(false) {}

std::shared_ptr<PresenceManager> PresenceManager::sharedManager() {
    if (!m_instance) m_instance = utils::protected_make_shared<PresenceManager>();
    return m_instance;
}

void PresenceManager::enableRPC(bool enable) {
    m_rpcIsEnabled = enable;
}

void PresenceManager::setActive(bool active) {
    m_rpcIsActive = active;
}