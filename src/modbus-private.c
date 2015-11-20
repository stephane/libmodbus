
#include"modbus-private.h"
#include<stdlib.h>

modbus_allocator_t allocators = {NULL,NULL,NULL,NULL};

modbus_set_default_allocators()
{
  if(!allocators.malloc)
    allocators.malloc = *malloc;
  if(!allocators.realloc)
    allocators.realloc = *realloc;
  if(!allocators.calloc)
    allocators.calloc = *calloc;
  if(!allocators.free)
    allocators.free = *free;
}