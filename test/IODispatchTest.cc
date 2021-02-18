#include "IODispacther.h"

#include <iostream>
#include <signal.h>
using namespace cocoa;
using namespace std;

class Signal : public IOSchedulable
{
public:
    Signal()
    {
        cout << "Constructor, this = " << this << endl;
    }
    ~Signal() override {}

    void handler(CallableArgs *arg)
    {
        cout << "handler, this = " << this << endl;
    }
};

int main(int argc, char const *argv[])
{
    OCInitialize();
    Signal sig;

    GOM->addObject(new IODispatcher());
    auto cb = std::bind(&Signal::handler, &sig, placeholders::_1);
    IODispatcher::instance()->newSignalCallable(&sig, &Signal::handler, set<int>{ SIGINT });

    while (1)
    {
        cout << "Re-schedule" << endl;
        IODispatcher::instance()->schedule();
    }

    OCFinalize();
    return 0;
}
