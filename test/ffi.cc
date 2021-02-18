#include <iostream>
#include <ffi.h>


class T
{
public:
    void method(const char *str)
    {
        std::cout << "method() called, str = " << str << std::endl;
    }
};

template<typename _Td, typename _Ts>
_Td ptr_cast(_Ts src)
{
    return *static_cast<_Td*>(static_cast<void*>(&src));
}

int main(int argc, char const *argv[])
{
    T obj;

    ::ffi_cif cif;
    ::ffi_type *args_types[2];
    void *args_values[2];

    const char *str = "Hello, World!";

    args_types[0] = &::ffi_type_pointer;
    args_types[1] = &::ffi_type_pointer;

    args_values[0] = &obj;
    args_values[1] = &str;

    ::ffi_prep_cif(&cif, ::ffi_abi::FFI_DEFAULT_ABI, 2, &::ffi_type_void, args_types);

    auto fn = ptr_cast<void(*)()>(&T::method);
    ::ffi_call(&cif, fn, nullptr, args_values);
    return 0;
}
