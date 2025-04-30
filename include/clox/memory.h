#ifndef clox_memory_h
#define clox_memory_h
#include "common.h"

#define CAPACITY_MIN 0x100

#define GROW_CAPACITY(capacity) \
    ((capacity) < (CAPACITY_MIN) ? (CAPACITY_MIN) : (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)reallocate(pointer, (oldCount) * sizeof(type), (newCount) * sizeof(type))

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, (oldCount) * sizeof(type), 0)

/* Can increase, decrease the size of arrays, and put the pointer to NULL */
void* reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif