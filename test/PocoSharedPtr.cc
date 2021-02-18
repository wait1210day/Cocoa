#include <iostream>
#include <Poco/SharedPtr.h>
using namespace std;

class T
{
public:
    T() { cout << "Constructor " << this << endl; }
    ~T() { cout << "Destructor " << this << endl; }
};

int main(int argc, char const *argv[])
{
    Poco::SharedPtr<T> t0(new T());
    t0.assign(new T());
    return 0;
}
