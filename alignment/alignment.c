#include "alignment.h"

size_t align(size_t size)
{
    size_t base = sizeof(long double);
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
