#include <iostream>

#include "moe/GCHeap.h"
#include "moe/Handle.h"
#include "moe/GarbageCollected.h"
using namespace cocoa;

class R;
class T : public moe::GarbageCollected<T>
{
public:
    T(int n) { std::cout << "Construct T@" << this << ", n = " << n << std::endl; }
    ~T() override { std::cout << "Destruct T" << std::endl; }

    void foo()
    {
        std::cout << "foo() is called" << std::endl;
    }

    void trace(moe::Visitor *visitor) override
    {
        visitor->trace(rHandle);
    }

private:
    moe::Handle<R>  rHandle;
};

class R : public moe::GarbageCollected<R>
{

};

class User : public moe::GarbageCollected<User>
{
public:
    User()
    {
        std::cout << "Construct User" << std::endl;
        handle = moe::Handle<T>::New(2233);
    }
    ~User() { std::cout << "Destruct User" << std::endl; }

    void foo()
    {
        handle->foo();
    }

    void trace(moe::Visitor *visitor) override
    {
        visitor->trace(handle);
    }

    moe::Handle<T> handle;
};

class Scoped
{
public:
    Scoped() { std::cout << "Enter scope" << std::endl; }
    ~Scoped() { std::cout << "Leave scope" << std::endl; }
};

void scope()
{
    Scoped scoped;

    moe::Local<User> user((User()));
    user->foo();
    std::cout << "Collecting in scope" << std::endl;
    moe::GetGCHeap()->collect();
    std::cout << "Finish collecting in scope" << std::endl;
}

int main()
{
    /* We have 16MB heap size */
    moe::CreateGCHeap(16 * 1024 * 1024);

    scope();
    std::cout << "Collecting" << std::endl;
    moe::GetGCHeap()->collect();
    std::cout << "Finish collecting" << std::endl;

    moe::DestroyGCHeap();
    return 0;
}
