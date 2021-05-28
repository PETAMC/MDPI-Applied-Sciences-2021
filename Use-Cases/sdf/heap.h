#ifndef HEAP_H_
#define HEAP_H_

#include <stddef.h>

typedef struct {
    void *nextfreeaddress;
} heap_t;

void InitMemory(heap_t *memory, void *startaddress);
void* AllocMemory(heap_t *memory, size_t size);

#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

