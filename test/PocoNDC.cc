#include <Poco/NestedDiagnosticContext.h>
#include <iostream>

void func()
{
    poco_ndc(func);
    Poco::NDC::current().dump(std::cout);
}

int main(int argc, char const *argv[])
{
    func();
    return 0;
}
