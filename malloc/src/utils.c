#include "malloc.h"

void *incr_void(void *base, size_t toadd, size_t mult)
{
    char *cast = base;
    cast = cast + toadd * mult;
    return cast;
}

size_t align(size_t size, size_t base)
{
    if (size % base == 0)
    {
        return size;
    }
    unsigned long res;
    if (__builtin_umull_overflow(base, (size / base) + 1, &res) != 0)
    {
        return 0;
    }
    return res;
}
