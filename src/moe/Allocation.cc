#include <list>
#include <malloc.h>

#include <mutex>

#include "moe/GCHeap.h"
#include "moe/Allocation.h"

MOE_NAMESPACE_BEGIN

HeapAllocator::Iterable::Iterable(std::list<HeapElement>& list)
    : fList(list)
{}

HeapAllocator::HeapAllocator(std::size_t maxHeapSize)
    : fMaxHeapSize(maxHeapSize),
      fHeapSizeInUse(0)
{
}

HeapAllocator::~HeapAllocator()
{
    for (HeapElement& element : fHeap)
        ::free(element.fPtr);
}

std::size_t HeapAllocator::freeHeapSize() const
{
    return fMaxHeapSize - fHeapSizeInUse;
}

void *HeapAllocator::allocate(std::size_t size)
{
    if (size == 0)
        return nullptr;

    if (size + fHeapSizeInUse > fMaxHeapSize)
        return nullptr;

    HeapElement heapElement{};
    heapElement.fSize = size;
    heapElement.fPtr = ::malloc(size);
    if (heapElement.fPtr == nullptr)
        return nullptr;

    fHeap.push_back(heapElement);
    fHeapSizeInUse += heapElement.fSize;

    return heapElement.fPtr;
}

void HeapAllocator::free(void *ptr)
{
    for (auto itr = fHeap.begin(); itr != fHeap.end(); itr++)
    {
        if (itr->fPtr == ptr)
        {
            fHeapSizeInUse -= itr->fSize;
            ::free(itr->fPtr);
            fHeap.erase(itr);
            break;
        }
    }
}

HeapAllocator::Iterable HeapAllocator::iterable()
{
    return Iterable(fHeap);
}

MOE_NAMESPACE_END
