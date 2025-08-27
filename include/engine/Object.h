#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <memory>

#define Event_Callback std::function<void(std::shared_ptr<void>)>
#define Update_Callback std::function<void(const long double)>

// This uses Object* instead of std::shared_ptr<Object> for updates as the Object is never deconstructed
class Object : public std::enable_shared_from_this<Object> {
public:
    inline bool isEqual(std::shared_ptr<Object> object) { return this == object.get(); }
    inline bool operator==(std::shared_ptr<Object> object) { return this == object.get(); };

    virtual bool init();
    virtual bool init(std::shared_ptr<Update_Callback> update);

    static std::shared_ptr<Object> create();

    static std::shared_ptr<Object> createWithUpdate(std::shared_ptr<Update_Callback> update);

    void setUpdate(std::shared_ptr<Update_Callback> update);
    // Run Scheduler::scheduleUpdate on self
    void scheduleSelf();
    // Run Scheduler::unscheduleUpdate on self
    void unscheduleSelf();
    virtual void update(const long double dt);

    // Name is case-insensitive
    void registerEventListener(std::string name, std::shared_ptr<Event_Callback> callback);
    // Name is case-insensitive
    void unregisterEventListener(std::string name, std::shared_ptr<Event_Callback> callback);

protected:
    Object();

    std::unordered_map<std::string, std::vector<std::shared_ptr<Event_Callback>>> m_callbacks;

    void _callEventListener(std::string name, std::shared_ptr<void> data  = nullptr);
    
    std::vector<std::shared_ptr<Event_Callback>> _getOrCreateListeners(std::string name);

private:
    std::shared_ptr<Update_Callback> m_updateCallback;
};