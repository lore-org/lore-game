#pragma once

#include <memory>
#include <vector>
#include <unordered_set>

#include <furredengine/Object.h>

namespace FurredEngine {

struct _entry {
    Object* target;
    int64_t priority;
    bool paused;
    bool willDelete;
};

class Scheduler : public Object {
public:
    virtual ~Scheduler();

    static std::shared_ptr<Scheduler> sharedScheduler();

    inline long double getTimeScale() { return m_timeScale; }
    void setTimeScale(long double timeScale);

    virtual void update(long double dt) override;

    void scheduleUpdate(Object* target, int64_t priority = 0, bool paused = false);

    void unscheduleUpdate(Object* target);
    void unscheduleUpdates(std::unordered_set<Object*> targets);
    void unscheduleAll();

    void pauseTarget(Object* target);
    void resumeTarget(Object* target);

    void pauseTargets(std::unordered_set<Object*> targets);
    void resumeTargets(std::unordered_set<Object*> targets);

    void pauseAll();
    void resumeAll();

    bool isTargetPaused(Object* target);

protected:
    Scheduler();

    long double m_timeScale;
    std::vector<std::shared_ptr<_entry>> m_entries;

private:
    static std::shared_ptr<Scheduler> m_instance;

    // Returns -1 if target does not exist
    int64_t _getIndexOfTarget(Object* target);
    
    // Returns nullptr if target does not exist
    std::shared_ptr<_entry> _getEntryFromTarget(Object* target);
};

}