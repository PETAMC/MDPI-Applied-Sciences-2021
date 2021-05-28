#include <heap.h>

/*
 * Memory Manager
 */

void InitMemory(heap_t *memory, void *startaddress)
{
    memory->nextfreeaddress = startaddress;
}

void* AllocMemory(heap_t *memory, size_t size)
{
    void *freemem = memory->nextfreeaddress;
    memory->nextfreeaddress += size;
    return freemem;
}


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

