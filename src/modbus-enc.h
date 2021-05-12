#ifndef MODBUS_ENC_H
#define MODBUS_ENC_H

#include "modbus.h"

MODBUS_BEGIN_DECLS

MODBUS_API modbus_t* modbus_new_enc(const char *ip_address, int port);

MODBUS_END_DECLS

#endif /* MODBUS_ENC_H */
