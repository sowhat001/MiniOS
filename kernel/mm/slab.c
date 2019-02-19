#include <arch.h>
#include <driver/vga.h>
#include <zjunix/slab.h>
#include <zjunix/utils.h>

// #define SLAB_DEBUG

#define KMEM_ADDR(PAGE, BASE) ((((PAGE) - (BASE)) << PAGE_SHIFT) | 0x80000000)

/*
 * one list of PAGE_SHIFT(now it's 12) possbile memory size
 * 96, 192, 8, 16, 32, 64, 128, 256, 512, 1024, (2 undefined)
 * in current stage, set (2 undefined) to be (4, 2048)
 */
struct kmem_cache kmalloc_caches[PAGE_SHIFT];

static unsigned int size_kmem_cache[PAGE_SHIFT] = { 8, 16, 32, 64, 96, 128, 192, 256, 512, 1024, 1536, 2048};

// init the struct kmem_cache_cpu
void init_kmem_cpu(struct kmem_cache_cpu *kcpu) {
    kcpu->page = 0;
}

// init the struct kmem_cache_node
void init_kmem_node(struct kmem_cache_node *knode) {
    #ifdef SLAB_DEBUG
    kernel_printf("init_kmem_node\n");
    #endif
    knode->nr_partial = 0;
    INIT_LIST_HEAD(&(knode->partial));
    int i;
    #ifdef SLAB_DEBUG
    kernel_printf("put in node partial\n");
    #endif

    // when doing initialization, put min_partial/2 into node_partial
    for(i=0;i<min_partial/2;i++){
        struct page *newpage;
        newpage = __alloc_pages(0);  // get bplevel = 0 page === one page
        if (!newpage) {
            kernel_printf("ERROR: slab request one page in cache failed\n");
        }
        struct kmem_cache * cache_ = container_of(knode, struct kmem_cache, node);
        format_slabpage(cache_ , newpage);
        list_add_tail(&(newpage->list), &(cache_->node.partial));
        cache_->cpu.page=0;
    }
}

void init_each_slab(struct kmem_cache *cache, unsigned int size) {
    cache->objsize = size;
    cache->size = UPPER_ALLIGN(size, SIZE_INT);  // add one char as mark(available)
    cache->offset = cache->objsize + sizeof(void *);
    init_kmem_cpu(&(cache->cpu));
    init_kmem_node(&(cache->node));
}

void init_slab() {
    unsigned int i;
    for (i = 0; i < PAGE_SHIFT; i++) {
        init_each_slab(&(kmalloc_caches[i]), size_kmem_cache[i]);
    }
}

// init slab page
void format_slabpage(struct kmem_cache *cache, struct page *page) {
    #ifdef SLAB_DEBUG
    kernel_printf("void format_slabpage(struct kmem_cache *cache, struct page *page)\n");
    #endif
    unsigned char *moffset = (unsigned char *)KMEM_ADDR(page, pages);  // physical addr
    set_flag(page, _PAGE_SLAB);

    cache->cpu.page = page;
    page->virtual = (void *)cache;
    page->freelist = moffset;
    page->bplevel = 0;

    // init the freelist
    unsigned int *ptr;
    unsigned int remaining = (1 << PAGE_SHIFT);
    #ifdef SLAB_DEBUG
    kernel_printf("init the freelist\n");
    #endif
    do {
        ptr = (unsigned int *)(moffset + cache->size);
        moffset += cache->offset;
        *ptr = (unsigned int)moffset;
        remaining -= cache->offset;
    } while (remaining >= cache->offset);
    // last ptr is NULL
    *ptr = 0;
}

// allocate memory for object
void *slab_alloc(struct kmem_cache *cache) {
    #ifdef SLAB_DEBUG
    kernel_printf("void *slab_alloc(struct kmem_cache *cache)\n");
    #endif

    void *object = 0;
    
    cpu_page:
    if(cache->cpu.page){ 
        #ifdef SLAB_DEBUG
        kernel_printf("\n");
        //kernel_clear_screen(31);
        #endif
        struct page *cpage = cache->cpu.page;
        // check if is full
        if (cpage->freelist==0){
            init_kmem_cpu(&(cache->cpu));
            goto node_page;
        }
        object = (void *)(cpage->freelist);
        // update linked list
        unsigned int *ptr = (unsigned int *)\
            (cpage->freelist + cache->size);
        cpage->freelist= (unsigned char *)*ptr;
        *ptr=0;
        // update cpage info
        cpage->bplevel++;
        return object;
    }
    
    node_page:
    if(!list_empty(&(cache->node.partial))){
        #ifdef SLAB_DEBUG
        kernel_printf("node_page\n");
        #endif
        //c->page = n->partial
        cache->cpu.page = container_of(cache->node.partial.next, struct page, list);
        list_del(cache->node.partial.next);
        cache->node.nr_partial--;
        goto cpu_page;
    }
    // new slab
    else{
        #ifdef SLAB_DEBUG
        kernel_printf("new slab\n");
        #endif
        struct page *newpage;
        newpage = __alloc_pages(0);  // get bplevel = 0 page === one page
        if (!newpage) {
            kernel_printf("ERROR: slab request one page in cache failed\n");
        }
        // using standard format to shape the new-allocated page,
        // set the new page to be cpu.page
        format_slabpage(cache, newpage);
        // as it's newly allocated no check may be need
        object = (void*)newpage->freelist;
        // update linked list
        unsigned int *ptr = (unsigned int *)(newpage->freelist + cache->size);
        newpage->freelist += cache->offset;
        *ptr=0;
        // update page info
        newpage->bplevel++;
        return object;
    }
}

// free the object on cache
void slab_free(struct kmem_cache *cache, void *object) {
    #ifdef SLAB_DEBUG
    kernel_printf("void slab_free(struct kmem_cache *cache, void *object)\n");
    #endif

    struct page *opage = pages + ((unsigned int)object >> PAGE_SHIFT);
    unsigned int *ptr;

    if (opage->bplevel==0) {
        kernel_printf("ERROR : slab_free error!\n");
        return;
    }

    object = (void*)((unsigned int)object | KERNEL_ENTRY);

    // full? put to node partial
    if(opage!=cache->cpu.page && opage->freelist==0){
        #ifdef SLAB_DEBUG
        kernel_printf("full? put to node partial\n");
        #endif
        /*list_del_init(&(opage->list));
        #ifdef SLAB_DEBUG
        kernel_printf("1\n");
        #endif*/
        list_add_tail(&(opage->list), &(cache->node.partial));
        #ifdef SLAB_DEBUG
        kernel_printf("1\n");
        #endif
        cache->node.nr_partial++;
        #ifdef SLAB_DEBUG
        kernel_printf("1\n");
        #endif
    }

    #ifdef SLAB_DEBUG
    kernel_printf("update freelist\n");
    #endif

    // update freelist
    ptr = (unsigned int *)(((unsigned char*)object) + cache->size); 
    *ptr = (unsigned int)(opage->freelist);
    opage->freelist=(unsigned char *)object;
    opage->bplevel--;

    // after free is empty (must be in node partial)
    if(opage!=cache->cpu.page && opage->bplevel==0){
        #ifdef SLAB_DEBUG
        kernel_printf("after free is empty\n");
        #endif
        if(cache->node.nr_partial >= min_partial){
            //kernel_printf("node.nr_partial %d\n", cache->node.nr_partial);
            list_del(&(opage->list));
            __free_pages(opage, 0);
            cache->node.nr_partial--;
        }
    }
    return;
}

// find the best-fit slab system for (size)
unsigned int get_slab(unsigned int size) {
    unsigned int itop = PAGE_SHIFT;
    unsigned int i;
    unsigned int bf_num = (1 << (PAGE_SHIFT - 1));  // half page
    unsigned int bf_index = PAGE_SHIFT;             // record the best fit num & index

    for (i = 0; i < itop; i++) {
        if ((kmalloc_caches[i].objsize >= size) && (kmalloc_caches[i].objsize < bf_num)) {
            bf_num = kmalloc_caches[i].objsize;
            bf_index = i;
        }
    }
    return bf_index;
}

// kernel malloc
void *kmalloc(unsigned int size) {
    struct kmem_cache *cache;
    unsigned int bf_index;

    if (!size){
        kernel_printf("ERROR: size false\n");
        return 0;
    }

    // if the size larger than the max size of slab system, then call buddy
    if (size > kmalloc_caches[PAGE_SHIFT - 2].objsize) {
        size = UPPER_ALLIGN(size, 1<<PAGE_SHIFT);
        return (void *)(KERNEL_ENTRY | (unsigned int)alloc_pages(size >> PAGE_SHIFT));
    }

    // else call slab to malloc
    bf_index = get_slab(size);
    if (bf_index >= PAGE_SHIFT) {
        kernel_printf("ERROR: No available slab\n");
        while (1)
            ;
    }
    return (void *)(KERNEL_ENTRY | (unsigned int)slab_alloc(&(kmalloc_caches[bf_index])));
}

// kernel free
void kfree(void *obj) {
    struct page *page;

    obj = (void *)((unsigned int)obj & (~KERNEL_ENTRY));
    page = pages + ((unsigned int)obj >> PAGE_SHIFT);
    // if is not use in slab, call buddy to free
    if (!(page->flag & _PAGE_SLAB)){
        #ifdef SLAB_DEBUG
        kernel_printf("kfree: not _PAGE_SLAB\n");
        #endif
        return free_pages((void *)((unsigned int)obj & ~((1 << PAGE_SHIFT) - 1)), page->bplevel);
    }
    // else call slab_free
    else{
        #ifdef SLAB_DEBUG
        kernel_printf("kfree: _PAGE_SLAB\n");
        #endif
        return slab_free(page->virtual, obj);
    }
}