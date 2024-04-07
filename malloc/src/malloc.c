#include "malloc.h"

static struct bucket *head = NULL;

__attribute__((visibility("default"))) void *malloc(size_t size)
{
    return allocator(size, &head);
}

__attribute__((visibility("default"))) void free(void *ptr)
{
    free_alloc(ptr, &head);
}

__attribute__((visibility("default"))) void *realloc(void *ptr, size_t size)
{
    struct bucket *it = head;
    while (it != NULL)
    {
        if (ptr >= it->page_beg
            && ptr < incr_void(it->page_beg, it->page_size, 1))
        {
            break;
        }
        it = it->next;
    }
    if (it == NULL)
    {
        return malloc(size);
        ;
    }

    void *toret = malloc(size);
    memcpy(toret, ptr, it->block_size);
    free(ptr);
    return toret;
}

__attribute__((visibility("default"))) void *calloc(size_t nmemb, size_t size)
{
    size_t res;
    if (__builtin_umull_overflow(nmemb, size, &res))
    {
        return NULL;
    }
    void *toret = malloc(res);
    memset(toret, 0, res);
    return toret;
}
