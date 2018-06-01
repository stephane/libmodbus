
#include <stdio.h>
#include <errno.h>

#include "modbus-tcp.h"


int main()
{
    modbus_t *ctx;
    int rc;

    ctx = modbus_new_tcp("127.0.0.1", 502);
    if (ctx == NULL) {
        rc = -1;
        goto out;
    }

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        rc = -2;
        goto out;
    }

    rc = 0;

  out:

    if (ctx != NULL) {
        modbus_free(ctx);
    }

    return rc;
}
