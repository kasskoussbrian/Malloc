#include <stdint.h>
#include <stdio.h>

void *beware_overflow(void *ptr, size_t nmemb, size_t size)
{
    size_t res;
    int t = __builtin_umull_overflow(nmemb, size, &res);
    if (t != 0)
    {
        return NULL;
    }
    char *ptrx = ptr;
    ptrx = ptrx + res;
    return ptrx;
}
