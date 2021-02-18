#include <iostream>
#include <typeinfo>
#include <concepts>
using namespace std;

template<typename T>
T expand_template() {
    cout << typeid(T).name() << endl;
    return T();
}

template<>
int expand_template() {
    cout << "int spec" << endl;
    return 1;
}

template<typename T>
concept IsPointer = is_pointer<T>::value;

template<IsPointer T>
T expand_template() {
    cout << "pointer spec" << endl;
    return nullptr;
}

template<typename... Args>
void expand()
{
    int arr[] = { (expand_template<Args>(), 1)... };
}

int main(int argc, char const *argv[])
{
    expand<int, int, double, bool, int *>();
    return 0;
}
