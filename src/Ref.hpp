#pragma once

class Ref {
protected:
    unsigned int m_refCount;

    Ref() : m_refCount(1) {};

public:
    virtual void retain() {
        ++m_refCount;
    };

    virtual void release() {
        --m_refCount;

        if (m_refCount <= 0)
            delete this;
    };
    
    virtual bool isSingleReference() {
        return m_refCount == 1;
    }
    /**
     * Returns the Ref's current reference count.
     *
     * @returns The Ref's reference count.
     */
    virtual unsigned int retainCount() {
        return m_refCount;
    };
};