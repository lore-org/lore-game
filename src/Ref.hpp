#pragma once

class Ref {
protected:
    int m_refCount;

    Ref() : m_refCount(1) {};

public:
    void retain() {
        ++m_refCount;
    };

    void release() {
        --m_refCount;

        if (m_refCount <= 0)
            delete this;
    };

    int getReferenceCount() {
        return m_refCount;
    };
};