/*
 * Copyright © 2018 Lars Täuber <gitaeuber@users.noreply.github.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef MODBUS_UCL_H
#define MODBUS_UCL_H

MODBUS_BEGIN_DECLS

const char *ucl_msg_type[] = {
    "msg_type=unknown,",
    "msg_type=request,",
    "msg_type=response,"
};

#define UCL_ADDR		        "addr="
#define UCL_BIT_FIELD		   "bit_field="
#define UCL_CHAR		        "char="
#define UCL_COUNT		       "count="
#define UCL_CRC			         "crc="
#define UCL_DATA		        "data="
#define UCL_EXCEPTION		   "exception="
#define UCL_FUNCTION		    "function="
#define UCL_GARBAGE		 "garbage_msg="
#define UCL_STATUS		      "status="
#define UCL_SUB_FUNCTION	"sub_function="

#define UCL_BUSY		"busy,"
#define UCL_IDLE		"idle,"
#define UCL_UNKNOWN		"unknown,"

#define UCL_ERROR_CRC		"error=crc,"
#define UCL_ERROR_DATA		"error=data,"
#define UCL_ERROR_FUNCTION	"error=function,"
#define UCL_ERROR_LENGTH	"error=length,"

MODBUS_END_DECLS

#endif  /* MODBUS_UCL_H */
