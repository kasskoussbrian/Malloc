#include "malloc.h"
static struct bucket *create_bucket(size_t size)
{
    // void *beg = mmap(NULL, align(400000000, 4096), PROT_READ | PROT_WRITE,
    //                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    void *beg = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (beg == MAP_FAILED)
    {
        //  fprintf(stderr "malloc: create_block : there is an error while
        //  mapping\n");
        return NULL;
    }

    //    void *meta_data = mmap(NULL, align(400000000, 4096), PROT_READ |
    //    PROT_WRITE,
    //                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    void *meta_data = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (meta_data == MAP_FAILED)
    {
        // fprintf(stderr,"malloc: create_block : there is an error while
        // mapping\n");
        return NULL;
    }
    struct bucket *toret = meta_data;

    // toret->meta_data_size = align(400000000, 4096);
    toret->meta_data_size = 4096;
    toret->block_size = size;
    toret->capacity = 0;
    toret->page_beg = beg;
    toret->headfree = NULL;
    toret->headchunk = NULL;
    toret->next = NULL;
    // toret->page_size = align(400000000, 4096);
    toret->page_size = 4096;
    toret->meta_data_beg = meta_data;
    toret->meta_data_write =
        incr_void(toret->meta_data_beg, sizeof(struct bucket), 1);

    return toret;
}

static struct bucket_iterator *create_iterator(struct bucket *bucket)
{
    if (bucket->capacity * bucket->block_size > bucket->page_size)
    {
        void *reso = mremap(
            bucket->page_beg, bucket->page_size,
            align(bucket->page_size + sizeof(struct bucket_iterator), 4096),
            MREMAP_MAYMOVE);
        if (reso == NULL)
        {
            return NULL;
        }
        bucket->page_beg = reso;
        bucket->page_size += 4096;
    }

    /*
    void *tmp1 = incr_void(bucket->meta_data_write,sizeof(struct
    bucket_iterator),1); void *tmp2 =
    incr_void(bucket->meta_data_beg,bucket->meta_data_size,1);

    if (tmp1 >=tmp2 )
    {

        void * rem =  mremap(bucket->meta_data_beg,
    bucket->meta_data_size,bucket->meta_data_size + 4096, MREMAP_MAYMOVE); if
    (rem ==MAP_FAILED)
        {
            return NULL;
        }

        bucket->meta_data_beg  = rem;

        bucket->meta_data_size += 4096;
    }
    */
    struct bucket_iterator *toret = bucket->meta_data_write;
    char *add = bucket->meta_data_write;
    add = add + sizeof(struct bucket_iterator);
    bucket->meta_data_write = add;
    toret->free = NULL;
    toret->next = bucket->headchunk;
    bucket->headchunk = toret;
    char *tt = bucket->page_beg;
    tt = tt + bucket->capacity * bucket->block_size;
    toret->chunk = tt;
    bucket->capacity = bucket->capacity + 1;
    return toret;
}

void *allocator(size_t size, struct bucket **head)
{
    size_t real_size = align(size, sizeof(long double));
    struct bucket *it = *head;
    while (it != NULL)
    {
        if (it->block_size == real_size)
        {
            break;
        }
        it = it->next;
    }
    if (it == NULL)
    {
        it = create_bucket(real_size);
        it->next = *head;
        *head = it;
    }
    struct bucket_iterator *buck_it = it->headfree;
    if (buck_it != NULL)
    {
        it->headfree = buck_it->free;
        buck_it->free = NULL;
    }
    else
    {
        buck_it = create_iterator(it);
    }
    return buck_it->chunk;
}

static void sous_free(void *tofree, struct bucket *it, struct bucket **head,
                      struct bucket *prv)
{
    struct bucket_iterator *buck_it = it->headchunk;
    struct bucket_iterator *tmp = NULL;
    while (buck_it != NULL)
    {
        if (buck_it->chunk == tofree)
        {
            if (tmp == NULL)
            {
                it->headchunk = buck_it->next;
            }
            else
            {
                tmp->next = buck_it->next;
            }
            buck_it->next = NULL;
            buck_it->free = it->headfree;
            it->headfree = buck_it;
            break;
        }
        tmp = buck_it;
        buck_it = buck_it->next;
    }
    if (buck_it == NULL)
    {
        // fprintf(stderr,"Malloc: free_alloc: Double free OR you gave a random
        // adress that is inside a %ld page\n",it->block_size);
        return;
    }
    if (it->headchunk == NULL)
    {
        if (prv != NULL)
        {
            prv->next = it->next;
        }
        else
        {
            *head = it->next;
        }
        int res_unmap_chunk = munmap(it->page_beg, it->page_size);
        if (res_unmap_chunk == -1)
        {
            //       fprintf(stderr,"malloc: free : Unmapping of chunk page
            //       failed \n");
            return;
        }
        res_unmap_chunk = munmap(it->meta_data_beg, it->meta_data_size);
        if (res_unmap_chunk == -1)
        {
            //         fprintf(stderr,"malloc: free : Unmapping of chunk page
            //         failed \n");
            return;
        }
    }
}

void free_alloc(void *tofree, struct bucket **head)
{
    if (tofree == NULL)
    {
        return;
    }
    struct bucket *it = *head;
    if (it == NULL)
    {
        //     fprintf(stderr,"Malloc: free_alloc : Nothing to free\n");
        return;
    }
    struct bucket *prv = NULL;
    while (it != NULL)
    {
        if (tofree >= it->page_beg
            && tofree < incr_void(it->page_beg, it->page_size, 1))
        {
            break;
        }
        prv = it;
        it = it->next;
    }
    if (it == NULL)
    {
        //       fprintf(stderr,et env LD_LIBRARY_PATH=."Malloc: free_alloc:
        //       adress given not found in your memory\n");
        return;
    }
    sous_free(tofree, it, head, prv);
}
