#ifndef COCOA_ALLOCATION_H
#define COCOA_ALLOCATION_H

#include <cstdint>
#include <list>
#include <mutex>

#include "moe/Moe.h"
MOE_NAMESPACE_BEGIN

/**
 * HeapAllocator records the state of a heap region.
 * We don't care the object layout, it's up to compiler.
 * @note HeapAllocator is thread-safe.
 */
class HeapAllocator
{
public:
    struct HeapElement
    {
        std::size_t     fSize;
        void           *fPtr;
    };

    class Iterable
    {
    public:
        explicit Iterable(std::list<HeapElement>& list);
        Iterable(const Iterable& other) = default;

        std::list<HeapElement>::iterator begin()
        { return fList.begin(); }

        std::list<HeapElement>::iterator end()
        { return fList.end(); }

    private:
        std::list<HeapElement>& fList;
    };

    explicit HeapAllocator(std::size_t maxHeapSize);
    ~HeapAllocator();

    std::size_t freeHeapSize() const;
    void *allocate(std::size_t size);
    void free(void *ptr);

    Iterable iterable();

private:
    std::size_t                     fMaxHeapSize;
    std::size_t                     fHeapSizeInUse;
    std::list<HeapElement>          fHeap;
};

MOE_NAMESPACE_END
#endif //COCOA_ALLOCATION_H
