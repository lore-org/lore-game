#include <furredengine/PresenceManager.h>

#include <memory>

#include <discord-rpc.hpp>

#include <furredengine/Engine.h>
#include <furredengine/Geometry.h>
#include <furredengine/utils.h>

using namespace FurredEngine;

std::shared_ptr<PresenceManager> PresenceManager::m_instance;

PresenceManager::PresenceManager() : m_isActive(false), m_restartOnFail(false) {}

std::shared_ptr<PresenceManager> PresenceManager::sharedManager() {
    if (!m_instance) m_instance = utils::protected_make_shared<PresenceManager>();
    return m_instance;
}

void PresenceManager::startRPC(std::string clientID) {
    LogInfo("Attempting to start Discord Presence...");
    discord::RPCManager::get()
        .setClientID(clientID)
        .onReady([this](auto) {
            m_isActive = true;
            LogInfo("Discord Presence initialised.");
        })
        .onErrored([this, clientID](auto, auto) {
            m_isActive = false;
            LogError("Discord Presence quit unexpectedly.");

            if (m_restartOnFail) this->startRPC(clientID);
        })
        .initialize();
}