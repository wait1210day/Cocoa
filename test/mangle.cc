#include <iostream>
#include <cxxabi.h>

namespace kks
{

char const *pName = "ss";

} // namespace kks


int main(int argc, char const *argv[])
{
    std::cout << typeid(kks::pName).name() << std::endl;
    return 0;
}
