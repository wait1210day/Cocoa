#include <stdio.h>

void fn(int *restrict ptr)
{
    printf("%d\n", *ptr);
}

int main(int argc, char const *argv[])
{
    int val = 2233;
    fn(&val);
    return 0;
}
