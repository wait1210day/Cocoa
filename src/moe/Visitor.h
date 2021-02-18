#ifndef COCOA_VISITOR_H
#define COCOA_VISITOR_H

#include <list>
#include <vector>

#include "moe/Moe.h"
#include "moe/Handle.h"
MOE_NAMESPACE_BEGIN
class BaseGarbageCollected;

class Visitor
{
public:
    using TracingList = std::vector<BaseGarbageCollected*>;

    Visitor() = default;
    ~Visitor() = default;

    class Iterable
    {
    public:
        explicit Iterable(TracingList& tracingList);

        TracingList::iterator begin();
        TracingList::iterator end();

    private:
        TracingList&    fTracingList;
    };

    Iterable tracingIterable();

    template<typename T>
    void trace(const Handle<T>& member)
    {
        fTracing.push_back(static_cast<BaseGarbageCollected*>(member.fPtr));
    }

private:
    TracingList     fTracing;
};

MOE_NAMESPACE_END
#endif //COCOA_VISITOR_H
