#include <iostream>

#include "core/Slot.h"

class T
{
public:
    T(int v) : val(v) {}

    void func(double m)
    {
        std::cout << val << ',' << m << std::endl;
    }

    int val;
};

void func(int a, double b)
{
    std::cout << a << ',' << b << std::endl;
}

int main()
{
    // auto slot = cocoa::FunctorSlot<decltype(&func), void, int, double>(func);

    // slot(nullptr, 2, 3.14);
    auto slot = cocoa::MemberFunctionSlot<T, decltype(&T::func), void, double>(&T::func);

    T obj(22);
    slot(&obj, 3.14);
    return 0;
}
