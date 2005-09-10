/* 
   Copyright (C) 2001-2005 Stéphane Raimbault <stephane.raimbault@free.fr>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

   These library of functions are designed to enable a program send and
   receive data from a device that communicates using the Modbus protocol.
*/

#ifndef _MODBUS_H_
#define _MODBUS_H_

#include <termios.h>
#include <arpa/inet.h>

#define MODBUS_TCP_PORT      502

#define HEADER_LENGTH_RTU      0
#define PRESET_QUERY_SIZE_RTU  6

#define HEADER_LENGTH_TCP      6
#define PRESET_QUERY_SIZE_TCP 12

#define CHECKSUM_SIZE_RTU      2
#define CHECKSUM_SIZE_TCP      0	

/* 8 + HEADER_LENGTH_TCP */
#define MIN_QUERY_SIZE        14

/* MIN_RESPONSE_LENGTH + MAX(MAX*) */
#define MAX_PACKET_SIZE      261

#define MAX_READ_STATUS      800
#define MAX_READ_HOLD_REGS   100
#define MAX_READ_INPUT_REGS  100
#define MAX_WRITE_COILS      800
#define MAX_WRITE_REGS       100

#define REPORT_SLAVE_ID_SIZE 75

/* Time out between trames in microsecond */
#define TIME_OUT_BEGIN_OF_TRAME 500000
#define TIME_OUT_END_OF_TRAME   500000

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* Protocol exceptions */
#define ILLEGAL_FUNCTION        -0x01
#define ILLEGAL_DATA_ADDRESS    -0x02
#define ILLEGAL_DATA_VALUE      -0x03
#define SLAVE_DEVICE_FAILURE    -0x04
#define SERVER_FAILURE          -0x04
#define ACKNOWLEDGE             -0x05
#define SLAVE_DEVICE_BUSY       -0x06
#define SERVER_BUSY             -0x06
#define NEGATIVE_ACKNOWLEDGE    -0x07
#define MEMORY_PARITY_ERROR     -0x08
#define GATEWAY_PROBLEM_PATH    -0x0A
#define GATEWAY_PROBLEM_TARGET  -0x0B

/* Local */
#define COMM_TIME_OUT           -0x0C
#define PORT_SOCKET_FAILURE     -0x0D
#define SELECT_FAILURE          -0x0E
#define TOO_MANY_DATAS          -0x0F
#define INVALID_CRC             -0x10
#define INVALID_EXCEPTION_CODE  -0x11

typedef enum { RTU, TCP } type_com_t;

typedef struct _modbus_param_t {
	/* Communication : RTU or TCP */
	type_com_t type_com;
	/* Device: "/dev/ttyS0" */
	char device[11];
	/* Bauds: 19200 */
	int baud_i;
	/* Parity: "even", "odd", "none" */
	char parity[5];
	/* Data bit */
	int data_bit;
	/* Stop bit */
	int stop_bit;
	/* Save old termios settings */
	struct termios old_tios;
	/* Descriptor (tty or socket) */
	int fd;
	/* Flag debug */
	int debug;
	/* IP address */
	char ip[16];
	/* Header length used for offset */
	int header_length;
	/* Checksum size RTU = 2 and TCP = 0 */
	int checksum_size;
} modbus_param_t;

/* All functions used for sending or receiving data return :
   - the numbers of values (bits or word) if success (0 or more)
   - less than 0 for exceptions errors
*/

/* Reads the boolean status of coils and sets the array elements in
   the destination to TRUE or FALSE */
int read_coil_status(modbus_param_t *mb_param, int slave,
		     int start_addr, int count, int *dest);

/* Same as read_coil_status but reads the slaves input table */
int read_input_status(modbus_param_t *mb_param, int slave,
		      int start_addr, int count, int *dest);

/* Read the holding registers in a slave and put the data into an
   array */
int read_holding_registers(modbus_param_t *mb_param, int slave,
			   int start_addr, int count, int *dest);


/* Read the input registers in a slave and put the data into an
   array */
int read_input_registers(modbus_param_t *mb_param, int slave,
			 int start_addr, int count, int *dest);

/* Turn on or off a single coil on the slave device */
int force_single_coil(modbus_param_t *mb_param, int slave,
		      int coil_addr, int state);

/* Sets a value in one holding register in the slave device */
int preset_single_register(modbus_param_t *mb_param, int slave,
			   int reg_addr, int value);

/* Takes an array of ints and sets or resets the coils on a slave
   appropriatly */
int force_multiple_coils(modbus_param_t *mb_param, int slave,
			 int start_addr, int coil_count, int *data);

/* Copy the values in an array to an array on the slave */
int preset_multiple_registers(modbus_param_t *mb_param, int slave,
			      int start_addr, int reg_count, int *data);

/* Returns some useful information about the modbus controller */
int report_slave_id(modbus_param_t *mb_param, int slave,
		    unsigned char *dest);

/* Initialises a parameters structure
   - device : "/dev/ttyS0"
   - baud :   19200
   - parity : "even", "odd" or "none" 
   - data_bits : 5, 6, 7, 8 
   - stop_bits : 1, 2
*/
void modbus_init_rtu(modbus_param_t *mb_param, char *device,
		     int baud, char *parity, int data_bit,
		     int stop_bit);
/* Initialises a parameters structure for TCP
   - ip : "192.168.0.5" */
void modbus_init_tcp(modbus_param_t *mb_param, char *ip_address);


/* This function sets up a serial port for RTU communications to
   modbus or a TCP connexion */
int modbus_connect(modbus_param_t *mb_param);

/* This function closes the serial port and restores the previous port
   configuration or close the TCP connexion */
void modbus_close(modbus_param_t *mb_param);

/* Set in debug mode */
void modbus_set_debug(modbus_param_t *mb_param, int boolean);

/* Useful for a 'daemon' */
void modbus_listen_tcp(modbus_param_t *mb_param);

/* Non implemented :
   - read_exception_status()
*/

/* Find out what a master is trying to ask this slave device */
int get_slave_query_tcp(modbus_param_t *mb_param, int *slave_addr, int *query,
			int *start_addr, int *point_count, int *data);

#endif  /* _MODBUS_H_ */
