/* Client for function modbus_read_device_identification
   Author: Tim Knecht <t.knecht@eckelmann.de>
*/
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <modbus.h>

/* Configuration of modbus device SECOP SLV 105N4627 on RS232 with RS485
   converter.
   https://www.secop.com/fileadmin/user_upload/technical-literature/operating-instructions/slv_controller_105n46xx-series_operating_instructions_02-2013_dess300a202.pdf
*/
#define   DEV_MODBUS      "/dev/COM4"
#define   MODBUS_SPEED    19200
#define   MODBUS_PARITY   'E'
#define   MODBUS_BYTES    8
#define   MODBUS_STOPBIT  1
#define   MODBUS_RTU_MODE MODBUS_RTU_RS232

/* Expected output (Slave ID 42): <<OUTPUT_END
read-device-identification-client
Trying slave_id 42...
Opening /dev/COM4 at 19200 bauds (E, 8, 1)
[2A][2B][0E][01][00][54][71]
Waiting for a confirmation...
<2A><2B><0E><01><01><00><00><03><00><05><53><45><43><4F><50><01><08><31><30><35><4E><34><36><32><37><02><05><30><31><2E><31><30><E4><78>
read_device_id returned: 31
function code: 2b, MEI type: e, Read Device ID code: 1, conformity level: 1
more_follows: 0, next_object_id: 0, number of objects: 3
1. Object ID: 0, length: 5, "SECOP"
2. Object ID: 1, length: 8, "105N4627"
3. Object ID: 2, length: 5, "01.10"
OUTPUT_END
*/
int main(int argc, const char *argv[])
{
    if (argc != 2) {
        printf("Usage: test-libmodbus <slave_id>\n");
        return 1;
    }
    int slave_id = atoi(argv[1]);

    printf("read-device-identification-client\nTrying slave_id %d...\n", slave_id);

    modbus_t *ctx = modbus_new_rtu(DEV_MODBUS, MODBUS_SPEED, MODBUS_PARITY,
                                             MODBUS_BYTES, MODBUS_STOPBIT);
    if (ctx == NULL)
    {
        printf("Unable to create the libmodbus context!\n");
        return 1;
    }

    modbus_set_debug(ctx, TRUE);

    if (modbus_connect(ctx) == -1)
    {
        printf("Connection failed: %s!\n", modbus_strerror(errno));
        modbus_free(ctx);
        return 2;
    }

    if (modbus_set_slave(ctx, slave_id) == -1)
    {
        printf("Error setting slave id: %s!\n", modbus_strerror(errno));
        modbus_close(ctx);
        modbus_free(ctx);
        return 3;
    }

    if (modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_MODE) == -1)
    {
        printf("Error setting serial mode: %s!\n", modbus_strerror(errno));
        modbus_close(ctx);
        modbus_free(ctx);
        return 4;
    }

    uint8_t response[260];
    const int response_size = sizeof(response);
    const int read_device_id_code = 1; /* Basic Device Identification (VendorName, ProductCode and MajorMinorRevision)*/
    int next_object_id = 0;
    int more_follows = 0xff;
    int rc;

    while (more_follows == 0xff) {
        rc = modbus_read_device_identification(ctx, read_device_id_code, next_object_id, response_size, response);
        if (rc == -1)
        {
            printf("Read device identification failed: %s!\n", modbus_strerror(errno));

            modbus_close(ctx);
            modbus_free(ctx);
            return 5;
        }
        printf("read_device_id returned: %d\n", rc);
        more_follows = response[4];
        next_object_id = response[5];
        int object_cnt = response[6];
        printf("function code: %x, MEI type: %x, Read Device ID code: %d, conformity level: %x\n",
                (int)response[0], (int)response[1], (int)response[2], (int)response[3]);
        printf("more_follows: %x, next_object_id: %x, number of objects: %d\n",
                more_follows, next_object_id, object_cnt);
        int offset = 7;
        for (int i = 0; i < object_cnt; i++) {
            int object_length = response[offset + 1];
            printf("%d. Object ID: %x, length: %d, \"%.*s\"\n", i + 1, (int)response[offset], object_length,
                 object_length, response + offset + 2);
            offset += object_length + 2;
        }
        /* Some devices needs some time before sending new request. */
        sleep(1);
    }

    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
