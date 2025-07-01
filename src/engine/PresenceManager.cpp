#include <engine/Default.h>

#include <engine/PresenceManager.h>

#include <memory>

#include <engine/utils.hpp>

using namespace utils;

std::shared_ptr<PresenceManager> PresenceManager::m_instance;

PresenceManager::PresenceManager() : m_rpcIsEnabled(false), m_rpcIsActive(false) {};

std::shared_ptr<PresenceManager> PresenceManager::sharedManager() {
    if (!m_instance) m_instance = protected_make_shared<PresenceManager>();
    return m_instance;
};

void PresenceManager::enableRPC(bool enable) {
    m_rpcIsEnabled = enable;
};

bool PresenceManager::isEnabled() {
    return m_rpcIsEnabled;
};

bool PresenceManager::isActive() {
    return m_rpcIsActive;
};

void PresenceManager::setActive(bool active) {
    m_rpcIsActive = active;
};