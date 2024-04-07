#include "recycler.h"

#include <stdlib.h>
struct recycler *recycler_create(size_t block_size, size_t total_size)
{
    if ((block_size % sizeof(size_t) != 0) || block_size == 0 || total_size == 0
        || total_size % block_size != 0)
    {
        return NULL;
    }
    struct recycler *recy = malloc(sizeof(struct recycler));
    if (recy == NULL)
    {
        return NULL;
    }
    recy->capacity = total_size / block_size;
    recy->block_size = block_size;
    recy->chunk = malloc(block_size * total_size);
    if (recy->chunk == NULL)
    {
        return NULL;
    }
    recy->free = recy->chunk;
    struct free_list *it = recy->chunk;
    for (size_t i = 0; i < recy->capacity - 1; i++)
    {
        it->next = it + block_size;
        it = it->next;
    }
    it->next = NULL;
    return recy;
}

void recycler_destroy(struct recycler *r)
{
    if (r == NULL)
    {
        return;
    }
    free(r->chunk);
    free(r);
}

void *recycler_allocate(struct recycler *r)
{
    if (r == NULL)
    {
        return NULL;
    }
    struct free_list *it = r->free;
    void *toret = it;
    if (it != NULL)
    {
        r->free = it->next;
    }
    return toret;
}
void recycler_free(struct recycler *r, void *block)
{
    if (block == NULL)
    {
        return;
    }
    struct free_list *head = r->free;
    r->free = block;
    struct free_list *now = block;
    now->next = head;
}
