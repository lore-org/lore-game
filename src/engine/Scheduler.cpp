#include <engine/Default.h>

#include <engine/Scheduler.h>

#include <engine/utils.hpp>

std::shared_ptr<Scheduler> Scheduler::m_instance;

Scheduler::Scheduler() : m_timeScale(1.f)  {};

Scheduler::~Scheduler() {
    Object::~Object();

    this->unscheduleAll();
};

std::shared_ptr<Scheduler> Scheduler::sharedScheduler() {
    if (!m_instance) m_instance = utils::protected_make_shared<Scheduler>();
    return m_instance;
};

float Scheduler::getTimeScale() {
    return m_timeScale;
};

void Scheduler::setTimeScale(float timeScale) {
    m_timeScale = timeScale;
};

void Scheduler::update(double dt) {
    if (m_timeScale != 1.f) dt *= m_timeScale;

    std::ranges::for_each(
        m_entries,
        [dt](std::shared_ptr<_entry> entry) {
            if (!entry->paused && !entry->willDelete) {
                entry->target->update(dt);
            }
        }
    );

    int _i = 0;
    std::ranges::for_each(
        m_entries,
        [this, &_i](std::shared_ptr<_entry> entry) {
            ++_i;
            if (entry->willDelete) m_entries.erase(m_entries.begin() + _i);
        }
    );

    Object::update(dt);
};

void Scheduler::scheduleUpdate(Object* target, int priority, bool paused) {
    if (this->_getIndexOfTarget(target) < 0) return;

    m_entries.push_back(std::make_shared<_entry>(target, priority, paused, false));
    std::ranges::sort(m_entries, [](std::shared_ptr<_entry> a, std::shared_ptr<_entry> b) {
        return a->priority < b->priority;
    });
};

void Scheduler::unscheduleUpdate(Object* target) {
    if (auto entry = this->_getEntryFromTarget(target)) entry->willDelete = true;
};

void Scheduler::unscheduleUpdates(std::unordered_set<Object*> targets) {
    std::ranges::for_each(
        targets,
        [this](Object* target) { this->unscheduleUpdate(target); }
    );
};

void Scheduler::unscheduleAll() {
    std::ranges::for_each(
        m_entries,
        [this](std::shared_ptr<_entry> entry) { this->unscheduleUpdate(entry->target); }
    );
};

void Scheduler::pauseTarget(Object* target) {
    if (auto entry = this->_getEntryFromTarget(target)) entry->paused = true;
};

void Scheduler::resumeTarget(Object* target) {
    if (auto entry = this->_getEntryFromTarget(target)) entry->paused = false;
};

void Scheduler::pauseTargets(std::unordered_set<Object*> targets) {
    std::ranges::for_each(
        targets,
        [this](Object* target) { this->pauseTarget(target); }
    );
};

void Scheduler::resumeTargets(std::unordered_set<Object*> targets) {
    std::ranges::for_each(
        targets,
        [this](Object* target) { this->resumeTarget(target); }
    );
};

void Scheduler::pauseAll() {
    std::ranges::for_each(
        m_entries,
        [](std::shared_ptr<_entry> entry) { entry->paused = true; }
    );
};

void Scheduler::resumeAll() {
    std::ranges::for_each(
        m_entries,
        [](std::shared_ptr<_entry> entry) { entry->paused = false; }
    );
};

bool Scheduler::isTargetPaused(Object* target) {
    auto entry = this->_getEntryFromTarget(target);
    return entry ? entry->paused : false;
};

size_t Scheduler::_getIndexOfTarget(Object* target) {
    auto find = std::ranges::find_if(
        m_entries,
        [&target](std::shared_ptr<_entry> entry) { return entry->target == target; }
    );

    if (find == m_entries.end()) return -1;
    else return find - m_entries.begin();
};

std::shared_ptr<_entry> Scheduler::_getEntryFromTarget(Object* target) {
    auto find = std::ranges::find_if(
        m_entries,
        [&target](std::shared_ptr<_entry> entry) { return entry->target == target; }
    );

    if (find != m_entries.end()) return *find;
    else return nullptr;
};