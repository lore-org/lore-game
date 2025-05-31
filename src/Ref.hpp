#pragma once

/**
 * Ref is used for reference count management. If a class inherits from Ref,
 * then it is easy to be shared in different places.
 * @js NA
 */
class Ref {
protected:
    // count of references
    unsigned int m_refCount;

    /**
     * Constructor
     *
     * The Ref's reference count is 1 after construction.
     */
    Ref() : m_refCount(1) {};

public:
    /**
     * Retains the ownership.
     *
     * This increases the Ref's reference count.
     *
     * @see release, autorelease
     */
    void retain() {
        ++m_refCount;
    };

    /**
     * Releases the ownership immediately.
     *
     * This decrements the Ref's reference count.
     *
     * If the reference count reaches 0 after the decrement, this Ref is
     * destructed.
     *
     * @see retain, autorelease
     */
    void release() {
        --m_refCount;

        if (m_refCount <= 0)
            delete this;
    };

    /**
     * Returns the Ref's current reference count.
     *
     * @returns The Ref's reference count.
     */
    unsigned int getReferenceCount() {
        return m_refCount;
    };
};