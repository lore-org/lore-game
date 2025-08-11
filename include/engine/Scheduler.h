#pragma once

#include <memory>
#include <vector>
#include <unordered_set>

#include "Object.h"

struct _entry {
    Object* target;
    long long priority;
    bool paused;
    bool willDelete;
};

class Scheduler : public Object {
public:
    virtual ~Scheduler();

    static std::shared_ptr<Scheduler> sharedScheduler();

    double getTimeScale();
    void setTimeScale(double timeScale);

    virtual void update(double dt) override;

    void scheduleUpdate(Object* target, long long priority = 0, bool paused = false);

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

    double m_timeScale;
    std::vector<std::shared_ptr<_entry>> m_entries;

private:
    static std::shared_ptr<Scheduler> m_instance;

    // Returns -1 if target does not exist
    long long _getIndexOfTarget(Object* target);
    
    // Returns nullptr if target does not exist
    std::shared_ptr<_entry> _getEntryFromTarget(Object* target);
};