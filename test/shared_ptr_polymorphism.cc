#include <memory>
#include <iostream>

class Base
{
public:
    virtual ~Base() {
        std::cout << "~Base()" << std::endl;
    }
};

class Derived : public Base
{
public:
    ~Derived() override {
        std::cout << "~Derived()" << std::endl;
    }
};

class Scoped
{
public:
    Scoped(const std::string& st) : fStack(st) {
        std::cout << "Enter " << st << std::endl;
    }
    ~Scoped() {
        std::cout << "Leave " << fStack << std::endl;
    }
private:
    std::string fStack;
};

void fn(const std::shared_ptr<Base>& base)
{
    Scoped __scope("fn");
    auto copy = base;
    std::cout << "use count = " << copy.use_count() << std::endl;
}

int main(int argc, char const *argv[])
{
    Scoped __scope("main");
    auto derived = std::make_shared<Derived>();
    fn(derived);
    return 0;
}
