#ifndef _ZJUNIX_SLAB_H
#define _ZJUNIX_SLAB_H

#include <zjunix/list.h>
#include <zjunix/buddy.h>

#define SIZE_INT 4
#define SLAB_AVAILABLE 0x0
#define SLAB_USED 0xff

#define min_partial 32

/*
 * slab pages is chained in this struct
 * @partial keeps the list of un-totally-allocated pages
 * @full keeps the list of totally-allocated pages
 */
struct kmem_cache_node {
    struct list_head partial;
    unsigned nr_partial; 
};

/*
 * current being allocated page unit
 */
struct kmem_cache_cpu {
    struct page *page;
};

struct kmem_cache {
    unsigned int objsize;           // The size of an object without meta data
    unsigned int size;              // The size of an object including meta data
    unsigned int offset;            // Free pointer offset
    struct kmem_cache_node node;
    struct kmem_cache_cpu cpu;
    unsigned char name[16];
};

void format_slabpage(struct kmem_cache *cache, struct page *page);

// extern struct kmem_cache kmalloc_caches[PAGE_SHIFT];
extern void init_slab();
extern void *kmalloc(unsigned int size);
extern void kfree(void *obj);

#endif
