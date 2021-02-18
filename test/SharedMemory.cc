#include <Poco/SharedMemory.h>
#include <iostream>
#include <cstring>

int main(int argc, char const *argv[])
{
    std::size_t siz = 4096UL * 4096 * 100;
    Poco::SharedMemory shm("cocoa.ipc", siz, Poco::SharedMemory::AM_WRITE);

    char *ptr = shm.begin();
    std::memset(ptr, 0xff, siz);
    
    std::cout << "Done" << std::endl;
    std::getchar();
    return 0;
}
