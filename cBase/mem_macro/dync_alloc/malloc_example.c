#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define BLOCK_SIZE sizeof(struct block_meta)

void *heap_start_addr = NULL;

struct block_meta {
    struct block_meta *prev;
    struct block_meta *next;
    int free;
    size_t size;
};

void get_heap_start_addr(){
    if(!heap_start_addr)
        heap_start_addr = sbrk(0);
}

void *get_last_block(){
    struct block_meta *blk_meta;

    blk_meta = heap_start_addr;
    while(blk_meta->next){
        blk_meta = blk_meta->next;
    }
    return blk_meta;
}

void *find_block(size_t size){
    struct block_meta *blk_meta;

    blk_meta = heap_start_addr;
    while(blk_meta){
        if(blk_meta->free)
            if(blk_meta->size >= size)
                break;
        blk_meta = blk_meta->next;
    }
    return blk_meta;
}

void *extend_heap(void *prev, size_t size){
    void *cur_brk;
    struct block_meta *cur_blk_meta, *prev_blk_meta;

    cur_brk = sbrk(size + BLOCK_SIZE);
    if(!cur_brk){
        return NULL;
    }
    else{
        cur_blk_meta = cur_brk;
        cur_blk_meta->free = 0;
        cur_blk_meta->size = size;
        cur_blk_meta->prev = prev;
        cur_blk_meta->next = NULL;

        if(prev){
            prev_blk_meta = prev;
            prev_blk_meta->next = cur_blk_meta;
        }

        return cur_brk + BLOCK_SIZE;
    }
}

void split_block(struct block_meta *blk_meta, size_t size){
    struct block_meta *free_blk_meta;

    if(size + BLOCK_SIZE < blk_meta->size){
        free_blk_meta = blk_meta + size + BLOCK_SIZE;
        free_blk_meta->prev = blk_meta;
        free_blk_meta->next = blk_meta->next;
        free_blk_meta->size = blk_meta->size - size - BLOCK_SIZE;
        free_blk_meta->free = 1;

        blk_meta->size = size;
        blk_meta->free = 0;
        blk_meta->next = free_blk_meta;
    }
    else{
        blk_meta->free = 0;
    }
}

void *rt_malloc(size_t size) {
    void *split_blk, *cur_brk, *ptr;
    struct block_meta *last_blk;

    if(!size)
        return NULL;

    // align with 8 bytes.
    if(size & 0x7){
        size = ((size >> 3) + 1) << 3;
    }

    get_heap_start_addr();
    cur_brk = sbrk(0);
    if(heap_start_addr == cur_brk){
        // First malloc, just extend the brk.
        ptr = extend_heap(NULL, size);
        if(ptr)
            return ptr;
        else
            return NULL;
    }
    else{
        split_blk = find_block(size);
        if(split_blk){
            // There is a free block, and we split it.
            split_block(split_blk, size);
            ptr = split_blk + BLOCK_SIZE;
            return ptr;
        }
        else{
            // No block is valid, extend the heap.
            last_blk = get_last_block();
            ptr = extend_heap(last_blk, size);
            if(ptr)
                return ptr;
            else
                return NULL;
        }
    }
}

void rt_free(void *ptr) {
    struct block_meta *cur_blk_meta, *prev_blk_meta, *next_blk_meta;

    if(!ptr)
        return ;

    cur_blk_meta = ptr - BLOCK_SIZE;
    cur_blk_meta->free = 1;

    next_blk_meta = cur_blk_meta->next;
    if(next_blk_meta){
        if(next_blk_meta->free){
            cur_blk_meta->next = next_blk_meta->next;
            cur_blk_meta->size += next_blk_meta->size + BLOCK_SIZE;
        }
    }

    prev_blk_meta = cur_blk_meta->prev;
    if(prev_blk_meta){
        if(prev_blk_meta->free){
            prev_blk_meta->next = cur_blk_meta->next;
            prev_blk_meta->size += cur_blk_meta->size + BLOCK_SIZE;
        }
    }
}

int main()
{
	short *arr_test = rt_malloc(100*sizeof(short));
	
	for(int icnt = 0; icnt < 100; icnt++)
	{
		arr_test[icnt] = icnt;
		printf("arr:%d\n",icnt);
	}
	
	rt_free(arr_test);
}
