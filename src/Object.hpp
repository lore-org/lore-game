#pragma once

class Object {
protected:
    // count of references
    unsigned int m_refCount;
public:
    Object() : m_refCount(1) {};
    virtual ~Object() {
        this->release();
    };

    void release() {
        --m_refCount;

        if (m_refCount <= 0)
            delete this;
    };
    void retain() {
        ++m_refCount;
    };
    bool isSingleReference() {
        return m_refCount == 1;
    }
    unsigned int retainCount() {
        return m_refCount;
    };

    virtual bool isEqual(Object* object) {
        return this == object;
    };
    virtual bool operator==(Object* object) {
        return this->isEqual(object);
    };

    virtual void update(float dt) {};
};