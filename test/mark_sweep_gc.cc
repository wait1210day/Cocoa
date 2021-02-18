#include <iostream>
#include <list>

class BaseGarbageCollected
{
public:
    virtual ~BaseGarbageCollected() = default;

    BaseGarbageCollected()
        : fMarked(false) {}

    void trace(BaseGarbageCollected *ptr)
    { fTracing.push_back(ptr); }

    std::list<BaseGarbageCollected*>& tracing()
    { return fTracing; }

    bool isMarked()
    { return fMarked; }

    void mark()
    { fMarked = true; }

    void unmark()
    { fMarked = false; }

private:
    bool fMarked = false;
    std::list<BaseGarbageCollected*>     fTracing;
};

class Collector
{
public:
    void newObject(BaseGarbageCollected *ptr, bool asRootObject = false)
    {
        std::cout << "Heap: new " << ptr << std::endl;
        fHeap.push_back(ptr);
        if (asRootObject)
        {
            std::cout << "Heap: RootObject " << ptr << std::endl;
            fRootObjects.push_back(ptr);
        }
    }

    void removeObject(BaseGarbageCollected *ptr)
    {
        std::cout << "Heap: delete " << ptr << std::endl;
        fHeap.remove(ptr);
        for (BaseGarbageCollected *obj : fRootObjects)
        {
            if (obj == ptr)
            {
                fRootObjects.remove(obj);
                break;
            }
        }
    }

    void collect()
    {
        mark(fRootObjects);
        sweep();
    }

private:
    void mark(std::list<BaseGarbageCollected*> traced)
    {
        for (BaseGarbageCollected *ptr : traced)
        {
            if (ptr->isMarked())
                continue;
            ptr->mark();
            if (ptr->tracing().size() != 0)
                mark(ptr->tracing());
        }
    }

    void sweep()
    {
        std::list<BaseGarbageCollected*> garbage;
        for (BaseGarbageCollected *heapObj : fHeap)
        {
            if (heapObj->isMarked())
                heapObj->unmark();
            else
                garbage.push_back(heapObj);
        }

        for (BaseGarbageCollected *garbageObj : garbage)
            delete garbageObj;
    }

private:
    std::list<BaseGarbageCollected*>     fRootObjects;
    std::list<BaseGarbageCollected*>     fHeap;
};

Collector collector;

template<typename T>
class GarbageCollected : public BaseGarbageCollected
{
public:
    ~GarbageCollected() override
    {
        collector.removeObject(this);
    }

protected:
    GarbageCollected()
    {
        collector.newObject(this);
    }
};

class Test : public GarbageCollected<Test>
{
public:
    Test()
    { std::cout << "Test() constructed" << std::endl; }

    ~Test() override
    { std::cout << "Test() destructed" << std::endl; }
};

int main()
{
    Test *ptr = new Test();
    collector.collect();
}
