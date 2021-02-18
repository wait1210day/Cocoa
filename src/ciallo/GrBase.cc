#include "ciallo/GrBase.h"

#include <cxxabi.h>

CIALLO_BEGIN_NS

std::string cxx_abi::resolve_name(const std::string& name)
{
    int status;
    char *demangled_name = abi::__cxa_demangle(name.c_str(), nullptr, nullptr, &status);
    if (status < 0 || demangled_name == nullptr)
        return std::string(name);
    
    std::string res(demangled_name);
    std::free(demangled_name);
    return res;
}

CIALLO_END_NS
