#pragma once

class Ref {
protected:
    unsigned int m_refCount;

    Ref() : m_refCount(1) {};
    virtual ~Ref() {
        this->release();
        delete this;
    }

public:
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
};