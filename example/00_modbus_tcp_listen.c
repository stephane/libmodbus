
#include <stdio.h>
#include <sys/select.h>
#include <errno.h>

#include "modbus-tcp.h"


int main()
{
    modbus_t *ctx;
    int server_socket;
    fd_set refset;
    int rc;

    /* To listen any addresses on port 502 */
    ctx = modbus_new_tcp(NULL, 502);
    if (ctx == NULL) {
        rc = -1;
        goto out;
    }

    /* Handle until 10 established connections */
    server_socket = modbus_tcp_listen(ctx, 10);

    /* Clear the reference set of socket */
    FD_ZERO(&refset);

    /* Add the server socket */
    FD_SET(server_socket, &refset);

    if (select(server_socket + 1, &refset, NULL, NULL, NULL) == -1) {
    }

    rc = 0;

  out:

    return rc;
}