#include "page_begin.h"

void *page_begin(void *ptr, size_t page_size)
{
    unsigned long res = (unsigned long)ptr & ~(page_size - 1);
    char *ret = NULL;
    ret = ret + res;
    return ret;
}
