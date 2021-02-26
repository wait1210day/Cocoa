#include <iostream>
#include <functional>

class SlotBase
{
public:
    virtual void invoke() = 0;
};

template<typename ArgsT>
class Slot : public SlotBase
{
public:
    Slot(std::function<void(ArgsT)> func)
        : fFunction(std::move(func)) {}

    void invoke() override
    {
    }

private:
    std::function<void(ArgsT)>  fFunction;
};