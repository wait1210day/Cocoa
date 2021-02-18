#include <iostream>
using namespace std;

class Tp
{
public:
    Tp() { std::cout << "Tp constructor called" << std::endl; }
    ~Tp() { std::cout << "Tp destructor called" << std::endl; }

    int s;
};

class Tv
{
public:
    Tv() { std::cout << "Tv constructor called " << this << std::endl; }
    ~Tv() { std::cout << "Tv destructor called " << this << std::endl; }

private:
    static Tp p;
};

Tp Tv::p;

int main(int argc, char const *argv[])
{
    Tv tv1, t2;
    return 0;
}

