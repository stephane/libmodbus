#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "unit-test.h"


static void* _server_start(void* userdata)
{
    return (void*)unit_test_server_run((unit_test_server_t*)userdata);
}

int main(int argc,char** argv)
{
    int rc = 0;
    unit_test_server_t server;
    server.use_backend = TCP;
    rc = unit_test_server_listen(&server);
    if(rc)goto error;

    // Start server thread
    {
        pthread_t server_thread;
        rc = pthread_create(&server_thread,NULL,_server_start,&server);
        if(rc)goto error;
        // Run Client
        rc = unit_test_client_start(TCP);
        if(rc)goto error;
        rc = pthread_join(server_thread,NULL);
        if(rc)
        {
            printf("%s/n",strerror(errno));
        }
    }
    error:
    // Handle Error
    return rc;
}
