#include "moe/Moe.h"
#include "moe/GarbageCollected.h"
#include "moe/Visitor.h"
MOE_NAMESPACE_BEGIN

Visitor::Iterable::Iterable(TracingList& tracingList)
    : fTracingList(tracingList)
{
}

Visitor::TracingList::iterator Visitor::Iterable::begin()
{
    return fTracingList.begin();
}

Visitor::TracingList::iterator Visitor::Iterable::end()
{
    return fTracingList.end();
}

Visitor::Iterable Visitor::tracingIterable()
{
    return Iterable(fTracing);
}

MOE_NAMESPACE_END
