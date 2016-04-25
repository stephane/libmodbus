#include "virtual-reply.h"

#include <stdlib.h>
#include <string.h>

void modbus_virtualize_mapping(modbus_vmapping_t* dest,
                               modbus_mapping_t* source)
{
    memset(dest, 0, sizeof(*dest));
    dest->app = source;
}
