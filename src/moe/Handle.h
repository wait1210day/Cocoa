#ifndef COCOA_HANDLE_H
#define COCOA_HANDLE_H

#include <new>

#include "moe/Moe.h"
#include "moe/GCHeap.h"
MOE_NAMESPACE_BEGIN

template<typename T>
class Handle
{
    friend class Visitor;
public:
    Handle() : fPtr(nullptr) {}

    template<typename... ArgsT>
    static Handle<T> New(ArgsT&&... args)
    {
        void *raw_ptr = GetGCHeap()->allocateHandle(sizeof(T));
        T *final_ptr = ::new(raw_ptr) T(std::forward<ArgsT>(args)...);
        final_ptr->trace(final_ptr->__GetVisitor());
        GetGCHeap()->afterHandleConstruct(static_cast<BaseGarbageCollected*>(final_ptr));
        return Handle<T>(final_ptr);
    }

    T *operator->()
    {
        return fPtr;
    }

private:
    explicit Handle(T *ptr) : fPtr(ptr) {}

private:
    T       *fPtr;
};

template<typename T>
class Local
{
public:
    explicit Local(T&& obj) : fLocalObject(std::move(obj))
    {
        GetGCHeap()->allocateLocal(static_cast<BaseGarbageCollected*>(&fLocalObject));
        fpObject = &fLocalObject;
        fLocalObject.trace(fLocalObject.__GetVisitor());
    }

    Local(const Local<T>& other) : fLocalObject(other.fLocalObject)
    {
        GetGCHeap()->allocateLocal(static_cast<BaseGarbageCollected*>(&fLocalObject));
        fpObject = &fLocalObject;
        fLocalObject.trace(fLocalObject.__GetVisitor());
    }

    Local(Local<T>&& other)  noexcept
        : fLocalObject(std::move(other.fLocalObject)),
          fpObject(other.fpObject)
    {
        other.fpObject = nullptr;
    }

    ~Local()
    {
        if (fpObject != nullptr)
            GetGCHeap()->releaseLocal(fpObject);
    }

    T *operator->()
    { return fpObject; }

private:
    T       fLocalObject;
    T      *fpObject;
};

MOE_NAMESPACE_END
#endif //COCOA_HANDLE_H
