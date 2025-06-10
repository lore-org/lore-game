#pragma once

#include <vector>
#include <unordered_set>
#include <algorithm>

#include "Object.hpp"

typedef struct _entry {
    Object* target;
    int priority;
    bool paused;
    bool willDelete;
} entry;

class Scheduler;

inline Scheduler* g_scheduler;

class Scheduler : public Object {
public:
    Scheduler() : m_timeScale(1.f) {};
    virtual ~Scheduler() {
        this->unscheduleAll();
        this->release();
        delete this;
    };

    static Scheduler* sharedScheduler() {
        if (!g_scheduler) g_scheduler =  new Scheduler();
        return g_scheduler;
    }

    inline float getTimeScale() {
        return m_timeScale;
    }
    inline void setTimeScale(float timeScale) {
        m_timeScale = timeScale;
    }

    void update(float dt) {
        if (m_timeScale != 1.f) dt *= m_timeScale;

        std::for_each(
            m_entries.begin(),
            m_entries.end(),
            [dt](_entry& entry) {
                if (!entry.paused && !entry.willDelete) {
                    entry.target->update(dt);
                }
            }
        );

        int _i = 0;
        std::for_each(
            m_entries.begin(),
            m_entries.end(),
            [this, &_i](_entry& entry) {
                ++_i;
                if (entry.willDelete) m_entries.erase(m_entries.begin() + _i);
            }
        );
    };

    void scheduleUpdate(Object* target, int priority = 0, bool paused = false) {
        if (this->_getIndexOfTarget(target) < 0) return;

        m_entries.push_back({ target, priority, paused, false });
        std::sort(m_entries.begin(), m_entries.end(), [](_entry& a, _entry& b) {
            return a.priority < b.priority;
        });
    };

    inline void unscheduleUpdate(Object* target) {
        if (auto entry = this->_getEntryFromTarget(target)) entry->willDelete = true;
    };
    inline void unscheduleUpdates(std::unordered_set<Object*> targets) {
        std::for_each(
            targets.begin(),
            targets.end(),
            [this](Object* target) { this->unscheduleUpdate(target); }
        );
    };
    inline void unscheduleAll() {
        std::for_each(
            m_entries.begin(),
            m_entries.end(),
            [this](_entry& entry) { this->unscheduleUpdate(entry.target); }
        );
    };

    inline void pauseTarget(Object* target) {
        if (auto entry = this->_getEntryFromTarget(target)) entry->paused = true;
    };
    inline void resumeTarget(Object* target) {
        if (auto entry = this->_getEntryFromTarget(target)) entry->paused = false;
    };

    inline void pauseTargets(std::unordered_set<Object*> targets) {
        std::for_each(
            targets.begin(),
            targets.end(),
            [this](Object* target) { this->pauseTarget(target); }
        );
    };
    inline void resumeTargets(std::unordered_set<Object*> targets) {
        std::for_each(
            targets.begin(),
            targets.end(),
            [this](Object* target) { this->resumeTarget(target); }
        );
    };

    inline void pauseAll() {
        std::for_each(
            m_entries.begin(),
            m_entries.end(),
            [](_entry& entry) { entry.paused = true; }
        );
    };
    inline void resumeAll() {
        std::for_each(
            m_entries.begin(),
            m_entries.end(),
            [](_entry& entry) { entry.paused = false; }
        );
    };

    inline bool isTargetPaused(Object* target) {
        auto entry = this->_getEntryFromTarget(target);
        return entry ? entry->paused : false;
    };

protected:
    float m_timeScale;

    std::vector<_entry> m_entries;

private:
    // returns -1 if target does not exist
    inline size_t _getIndexOfTarget(Object* target) {
        auto find = std::find_if(
            m_entries.begin(),
            m_entries.end(),
            [&target](_entry& entry) { return entry.target == target; }
        );

        if (find == m_entries.end()) return -1;
        else return find - m_entries.begin();
    }
    
    // returns nullptr if target does not exist
    inline _entry* _getEntryFromTarget(Object* target) {
        auto find = std::find_if(
            m_entries.begin(),
            m_entries.end(),
            [&target](_entry& entry) { return entry.target == target; }
        );

        if (find != m_entries.end()) return &*find;
        else return nullptr;
    }
};