#include <iostream>

#include "Exception.h"
using namespace cocoa;

void fn()
{
    throw RuntimeException::Builder(__FUNCTION__)
            .append("Test exception: ")
            .append(2233)
            .make<RuntimeException>();
}

int main()
{
    try
    {
        fn();
    }
    catch (const RuntimeException& e)
    {
        std::cout << e.who() << std::endl;
        std::cout << e.what() << std::endl;
        for (const RuntimeException::Frame& f : e.frames())
        {
            std::cout << f.pc << " (" << f.procAddress << '+' << f.offset << ") "
                      << f.symbol << " at " << f.file << std::endl;
        }
    }
    return 0;
}
