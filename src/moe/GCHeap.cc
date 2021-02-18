#include <cstdint>
#include <stdexcept>
#include <mutex>

#include "moe/Allocation.h"
#include "moe/GCHeap.h"
#include "moe/GarbageCollected.h"

MOE_NAMESPACE_BEGIN

GCHeap  *programGCHeap = nullptr;

void CreateGCHeap(std::size_t maxHeapSize)
{
    if (programGCHeap != nullptr)
        throw std::runtime_error("MoeGC: Multi GCHeap is not allowed");

    programGCHeap = new GCHeap(maxHeapSize);
    if (programGCHeap == nullptr)
        throw std::runtime_error("MoeGC: Failed to allocate heap for GC");
}

void DestroyGCHeap()
{
    delete programGCHeap;
}

GCHeap *GetGCHeap()
{
    if (programGCHeap == nullptr)
        throw std::runtime_error("MoeGC: No available GCHeap");
    return programGCHeap;
}

// --------------------------------------------------------------------

GCHeap::GCHeap(std::size_t maxHeapSize)
    : fHeapAllocator(maxHeapSize)
{
}

GCHeap::~GCHeap()
{
    for (BaseGarbageCollected *basePtr : fHeapHandles)
        releaseHandle(basePtr, false);
}

void *GCHeap::allocateHandle(std::size_t size)
{
    std::scoped_lock<std::mutex> scopedLock(fHeapMutex);

    void *ptr = fHeapAllocator.allocate(size);
    if (ptr == nullptr)
    {
        this->collect();
        ptr = fHeapAllocator.allocate(size);
        if (ptr == nullptr)
            throw std::runtime_error("MoeGC: Out of heap memory");
    }
    return ptr;
}

void GCHeap::afterHandleConstruct(BaseGarbageCollected *handle)
{
    std::scoped_lock<std::mutex> scopedLock(fHeapMutex);
    if (handle == nullptr)
        return;
    fHeapHandles.push_back(handle);
}

void GCHeap::releaseHandle(BaseGarbageCollected *basePtr, bool removeFromHeapHandles)
{
    std::scoped_lock<std::mutex> scopedLock(fHeapMutex);

    if (basePtr == nullptr)
        return;

    void *finalPtr = basePtr->__FinalPtr();
    basePtr->~BaseGarbageCollected();
    fHeapAllocator.free(finalPtr);
    if (removeFromHeapHandles)
        fHeapHandles.remove(basePtr);
}

void GCHeap::allocateLocal(BaseGarbageCollected *ptr)
{
    std::scoped_lock<std::mutex> scopedLock(fLocalMutex);
    fRootSet.push_back(ptr);
}

void GCHeap::releaseLocal(BaseGarbageCollected *ptr)
{
    std::scoped_lock<std::mutex> scopedLock(fLocalMutex);
    fRootSet.remove(ptr);
}

void GCHeap::collect()
{
    std::scoped_lock<std::mutex> scopedHeapLock(fHeapMutex);
    std::scoped_lock<std::mutex> scopedLocalLock(fLocalMutex);

    for (BaseGarbageCollected *root : fRootSet)
        mark(root);
    sweep();
}

void GCHeap::mark(BaseGarbageCollected *node)
{
    if (node->__IsMarked())
        return;

    node->__SetMark(true);
    for (BaseGarbageCollected *ptr : node->__GetVisitor()->tracingIterable())
        mark(ptr);
}

void GCHeap::sweep()
{
    std::vector<BaseGarbageCollected*> sweepList;
    for (BaseGarbageCollected *ptr : fHeapHandles)
    {
        if (ptr->__IsMarked())
            ptr->__SetMark(false);
        else
            sweepList.push_back(ptr);
    }

    for (BaseGarbageCollected *sweepPtr : sweepList)
        this->releaseHandle(sweepPtr);
}

std::size_t GCHeap::freeHeapSize()
{
    return fHeapAllocator.freeHeapSize();
}

MOE_NAMESPACE_END
