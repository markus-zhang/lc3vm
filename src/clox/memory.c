#include <stdlib.h>
#include <stdio.h>

#include "memory.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize)
{
    if (oldSize > 0 && newSize == 0)
    {
        free(pointer);
        return NULL;
    }
    
    /* 
        realloc() fits all 3 conditions:
        - oldSize = 0 (pointer = NULL) && newSize > 0
        - oldSize > 0 && newSize < oldSize
        - oldSize > 0 && newSize > oldSize 
    */
    void* temp = realloc(pointer, newSize);
    if (temp == NULL)
    {
        printf("At %s, line %d: realloc() failed\n", __FILE__, __LINE__);
        exit(1);
    }
    /* Note that pointer is not touched */
    return temp;
}