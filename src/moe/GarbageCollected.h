#ifndef COCOA_GARBAGECOLLECTED_H
#define COCOA_GARBAGECOLLECTED_H

#include "moe/Moe.h"
#include "moe/Visitor.h"
MOE_NAMESPACE_BEGIN

class BaseGarbageCollected
{
public:
    BaseGarbageCollected()
        : fMark(false) {}
    virtual ~BaseGarbageCollected() = default;

    virtual void *__FinalPtr() = 0;

    inline bool __IsMarked()
    { return fMark; }

    inline void __SetMark(bool value)
    { fMark = value; }

    inline Visitor *__GetVisitor()
    { return &fVisitor; }

    /* Inherited class must implement this */
    virtual void trace(Visitor *visitor) = 0;

private:
    bool        fMark;
    Visitor     fVisitor;
};

template<typename T>
class GarbageCollected : public BaseGarbageCollected
{
public:
    ~GarbageCollected() override = default;
    void *__FinalPtr() override {
        return dynamic_cast<T*>(this);
    }
};

MOE_NAMESPACE_END
#endif //COCOA_GARBAGECOLLECTED_H
