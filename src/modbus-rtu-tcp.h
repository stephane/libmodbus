#ifndef MODBUS_RTU_TCP_H
#define MODBUS_RTU_TCP_H

#include "modbus.h"

MODBUS_BEGIN_DECLS

MODBUS_API modbus_t* modbus_new_rtu_tcp(const char *ip_address, int port);

MODBUS_END_DECLS

#endif /* MODBUS_TCP_H */
