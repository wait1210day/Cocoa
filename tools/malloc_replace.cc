#include <cstdlib>

bool enable_replace = false;
void malloc_replace_set(bool enable)
{
    enable_replace = enable;
}

extern "C" {

void *malloc(size_t size)
{
}

}