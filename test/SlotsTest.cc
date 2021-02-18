#include "QObject.h"
#include <iostream>
using namespace cocoa;
using namespace std;

class T : public QObject
{
public:
    T() {
        p = 2233;
    }

    void slot() {
        cout << "p = " << p << endl;
    }

    int p;
};

int main(int argc, char const *argv[])
{
    T *obj = new T();
    int s = 123;
    QObject::signal(nullptr, "@click");
    QObject::connect(obj, &T::slot, "@click");
    delete obj;
    QObject::emit("@click");
    return 0;
}
