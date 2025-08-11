#include <engine/PresenceManager.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <discord-rpc.hpp>

#include <engine/config.hpp>
#include <engine/Engine.h>
#include <engine/Geometry.h>
#include <engine/utils.hpp>

#include <memory>

using namespace utils;

std::shared_ptr<PresenceManager> PresenceManager::m_instance;

PresenceManager::PresenceManager() : m_rpcIsEnabled(false), m_rpcIsActive(false) {}

std::shared_ptr<PresenceManager> PresenceManager::sharedManager() {
    if (!m_instance) m_instance = protected_make_shared<PresenceManager>();
    return m_instance;
}

void PresenceManager::enableRPC(bool enable) {
    m_rpcIsEnabled = enable;
}

bool PresenceManager::isEnabled() {
    return m_rpcIsEnabled;
}

bool PresenceManager::isActive() {
    return m_rpcIsActive;
}

void PresenceManager::setActive(bool active) {
    m_rpcIsActive = active;
}