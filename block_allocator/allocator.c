#include "allocator.h"

#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
static size_t align(size_t size, size_t base)
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

struct blk_allocator *blka_new(void)
{
    struct blk_allocator *toret = malloc(sizeof(struct blk_allocator));
    if (toret == NULL)
    {
        return NULL;
    }
    toret->meta = NULL;
    return toret;
}

struct blk_meta *blka_alloc(struct blk_allocator *blka, size_t size)
{
    size_t totsize =
        align(size + sizeof(struct blk_meta), sysconf(_SC_PAGESIZE));
    struct blk_meta *toret = mmap(NULL, totsize, PROT_READ | PROT_WRITE,
                                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (toret == MAP_FAILED)
    {
        return NULL;
    }
    toret->size = totsize - sizeof(struct blk_meta);
    toret->next = blka->meta;
    blka->meta = toret;
    return toret;
}

void blka_free(struct blk_meta *blk)
{
    if (blk == NULL)
    {
        return;
    }
    munmap(blk, blk->size + sizeof(struct blk_meta));
}
void blka_pop(struct blk_allocator *blka)
{
    if (blka == NULL)
    {
        return;
    }
    struct blk_meta *tofree = blka->meta;
    if (tofree == NULL)
    {
        return;
    }
    blka->meta = blka->meta->next;
    blka_free(tofree);
}
void blka_delete(struct blk_allocator *blka)
{
    struct blk_meta *it = blka->meta;
    struct blk_meta *tofree;
    while (it != NULL)
    {
        tofree = it;
        it = it->next;
        blka_free(tofree);
    }
    free(blka);
}
