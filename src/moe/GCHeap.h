#ifndef COCOA_GCHEAP_H
#define COCOA_GCHEAP_H

#include <cstdint>
#include <map>
#include <stdexcept>
#include <mutex>

#include "moe/Moe.h"
#include "moe/Allocation.h"
MOE_NAMESPACE_BEGIN
class BaseGarbageCollected;

class GCHeap
{
public:
    explicit GCHeap(std::size_t maxHeapSize);
    ~GCHeap();

    void *allocateHandle(std::size_t size);
    void afterHandleConstruct(BaseGarbageCollected *handle);

    void allocateLocal(BaseGarbageCollected *ptr);
    void releaseLocal(BaseGarbageCollected *ptr);

    std::size_t freeHeapSize();
    std::size_t totalHeapSize();

    void collect();

private:
    void releaseHandle(BaseGarbageCollected *basePtr, bool removeFromHeapHandles = true);
    void mark(BaseGarbageCollected *node);
    void sweep();

private:
    HeapAllocator                       fHeapAllocator;
    std::list<BaseGarbageCollected*>    fRootSet;
    std::list<BaseGarbageCollected*>    fHeapHandles;
    std::mutex                          fLocalMutex;
    std::mutex                          fHeapMutex;
};

/**
 * @brief Create a GC instance (heap).
 */
void CreateGCHeap(std::size_t maxHeapSize);

/**
 * @brief Destroy current GC instance (heap).
 * @note When you destroy a GC heap,
 * all the objects in this heap will be deleted.
 */
void DestroyGCHeap();

GCHeap *GetGCHeap();

/**
 * @brief Get the size of heap system heap (allocated by system kernel).
 */
std::size_t GetSystemHeapSize();

MOE_NAMESPACE_END
#endif // COCOA_GCHEAP_H
