#include <iostream>
#include "Core/UniquePersistent.h"

class Foo : public cocoa::UniquePersistent<Foo>
{
public:
    void fuck()
    {
        std::cout << "Foo" << std::endl;
    }
};

class Bar : public cocoa::UniquePersistent<Bar>
{
public:
    Bar(int a, int b)
        : fa(a), fb(b) {}

    void fuck()
    {
        std::cout << "Bar " << fa << ',' << fb << std::endl;
    }

private:
    int fa;
    int fb;
};

int main()
{
    Foo::New();
    Bar::New(2233, 666);

    Foo::Instance()->fuck();
    Bar::Instance()->fuck();

    Foo::Delete();
    Bar::Delete();
    return 0;
}
