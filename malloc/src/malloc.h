#ifndef MALLOC_H
#define MALLOC_H
#define _GNU_SOURCE
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

struct bucket
{
    size_t block_size;
    size_t capacity;
    size_t page_size;

    struct bucket_iterator *headfree;
    struct bucket_iterator *headchunk;

    struct bucket *next;
    void *page_beg;

    void *meta_data_beg;
    void *meta_data_write;
    size_t meta_data_size;
};

struct bucket_iterator
{
    void *chunk;
    struct bucket_iterator *free;
    struct bucket_iterator *next;
};

size_t align(size_t size, size_t base);
void *incr_void(void *base, size_t toadd, size_t mult);
void *allocator(size_t size, struct bucket **head);
void free_alloc(void *tofree, struct bucket **head);

#endif /* ! MALLOC_H */
