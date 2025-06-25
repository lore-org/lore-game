#pragma once
#include "Default.hpp" // IWYU pragma: keep

#include <string>
#include <typeinfo>

class Ref {
public:
    inline operator std::string() const {
        return fmt::format("( {}, {} )", typeid(this).name(), fmt::ptr(this));
    }

    inline virtual void retain() {
        ++m_refCount;
    };

    virtual void release() {
        if (--m_refCount <= 0)
            delete this;
    };
    
    inline virtual bool isSingleReference() {
        return m_refCount == 1;
    }
    /**
     * Returns the Ref's current reference count.
     *
     * @returns The Ref's reference count.
     */
    inline virtual unsigned int retainCount() {
        return m_refCount;
    };

protected:
    Ref() : m_refCount(1) {};
    
    int m_refCount;
};