/*
 * Copyright © 2016 DEIF A/S
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#include <modbus.h>

#define ASSERT_VAL(_prefix, _cond) {   \
    printf(" - %s",_prefix);           \
    if (_cond) {                       \
        printf("OK\n");                \
        ok_cntr++;                     \
    } else {                           \
        printf("ERROR\n");             \
        perror("errno");               \
        err_cntr++;                    \
    }                                  \
};

#define PRINT_HEADER() printf("\n%s\n",__FUNCTION__);

#define PRINT_FOOTER() {                                                 \
   if(err_cntr == 0) {                                                   \
      printf(" # TEST PASSED, OK:%2d, ERROR:%2d\n", ok_cntr, err_cntr);  \
      return 0;                                                          \
   }                                                                     \
   else {                                                                \
      printf(" # TEST FAILED, OK:%2d, ERROR:%2d\n", ok_cntr, err_cntr);  \
      return -1;                                                         \
   }                                                                     \
};

#define MAX_CONNECTIONS 10
#define MB_MAP_BITS 5
#define MB_MAP_INPUT_BITS 10
#define MB_MAP_REGISTERS 15
#define MB_MAP_INPUT_REGISTERS 20
#define TEST_PORT 503

struct mb_server_data {
   modbus_tcp_server_t* mb_srv_ctx;
   modbus_mapping_t* mb_map;
};

static void* mb_server_thread(void* param) {
   struct mb_server_data* data = (struct mb_server_data*)param;

   //printf(" - INFO: modbus_server_thread_starting\n");
   while(modbus_tcp_server_handle(data->mb_srv_ctx, data->mb_map) == 0) {
      //handling
   }
   //printf(" - INFO: modbus_server_thread_stopping\n");
   return NULL;
}

static int test_start_modbus_server(struct mb_server_data* mb_param) {

   int err_cntr = 0;
   int ok_cntr  = 0;
   int rc = 0;
   int time_before = 0;
   pthread_t id;

   PRINT_HEADER();

   /* start context */
   mb_param->mb_srv_ctx = modbus_tcp_server_start("127.0.0.1", TEST_PORT, MAX_CONNECTIONS);
   ASSERT_VAL("create_modbus_server...", mb_param->mb_srv_ctx != NULL);

   /* create mapping */
   mb_param->mb_map = modbus_mapping_new(
         MB_MAP_BITS,MB_MAP_INPUT_BITS,MB_MAP_REGISTERS,MB_MAP_INPUT_REGISTERS);
   ASSERT_VAL("create_modbus_mapping...", mb_param->mb_map != NULL);

   /* Check select timeout before we launch a dedicated task */
   time_before = time(NULL);
   modbus_tcp_server_set_select_timeout(mb_param->mb_srv_ctx, 2, 0);

   /* Should sleep 2 sec */
   rc = modbus_tcp_server_handle(mb_param->mb_srv_ctx, mb_param->mb_map);

   ASSERT_VAL("verify_modbus_tcp_server_set_select_timeout...",
         ((rc == 0) && ((time(NULL) - time_before) >= 2)));

   /* set back to blocking */
   modbus_tcp_server_set_select_timeout(mb_param->mb_srv_ctx,MB_TCP_SRV_BLOCKING_TIMEOUT,0);

   /* spawn server thread */
   pthread_create(&id, NULL, mb_server_thread, (void*)mb_param);
   usleep(1000000);

   PRINT_FOOTER();
}


static int test_multiple_connections_to_modbus_server(struct mb_server_data* mb_param) {

   int err_cntr = 0;
   int ok_cntr  = 0;
   int i = 0;
   int rc = 0;
   uint16_t mb_register = 0;
   modbus_t* mb_cli_list[MAX_CONNECTIONS + 1] = {NULL};
   modbus_t* mb_cli = NULL;

   PRINT_HEADER();

   /* Create many connections to server */
   for(i = 0; i < MAX_CONNECTIONS +1; i++) {
      mb_cli_list[i] = modbus_new_tcp("127.0.0.1",TEST_PORT);
      if(mb_cli_list[i] != NULL) {
         if(modbus_connect(mb_cli_list[i]) == 0) {
            rc ++;
            usleep(10000); // modbus server only allows 5 incoming telegrams in receive queue, small delay
         }
      }
   }
   ASSERT_VAL("create_max+1_amount_of_modbus_clients...", rc == 11);

   mb_cli = mb_cli_list[0];
   rc = modbus_read_registers(mb_cli, 0, 1, &mb_register);
   ASSERT_VAL("verify_first_connect_was_closed_because_we_opend_one_too_many...", rc == -1);

   mb_cli = mb_cli_list[1];
   rc = modbus_read_registers(mb_cli, 0, 1, &mb_register);
   ASSERT_VAL("verify_second_connection_works...", rc == 1);

   mb_cli = mb_cli_list[MAX_CONNECTIONS];
   rc = modbus_read_registers(mb_cli, 0, 1, &mb_register);
   ASSERT_VAL("verify_last_connection_works...", rc == 1);

   /* Close connections to server */
   for(i = 0; i < MAX_CONNECTIONS + 1; i++) {
      modbus_free(mb_cli_list[i]);
   }

   PRINT_FOOTER();
}

static int test_read_write_to_modbus_server(void) {
   int err_cntr = 0;
   int ok_cntr  = 0;
   int rc = 0;
   uint16_t mb_register = 0;
   uint8_t mb_bit = 0;
   modbus_t* mb_cli = NULL;

   PRINT_HEADER();

   mb_cli = modbus_new_tcp("127.0.0.1",TEST_PORT);
   modbus_connect(mb_cli);

   /* Write registers and bits */
   rc = 0;
   rc += modbus_write_register(mb_cli, 0, 0xA5A5);  //OK
   rc += modbus_write_register(mb_cli, MB_MAP_REGISTERS-1, 0xA5A5); //OK
   rc += modbus_write_bit(mb_cli, 0, 1);            //OK
   rc += modbus_write_bit(mb_cli, MB_MAP_BITS-1, 1);            //OK
   ASSERT_VAL("write_to_correct_register_on_server...", rc == 4);

   rc = 0;
   rc += modbus_write_register(mb_cli, 15, 0xA5A5); //FAIL
   rc += modbus_write_bit(mb_cli, 5, 1);            //FAIL
   ASSERT_VAL("write_to_faulty_register_on_server...", rc == -2);

   /* Read registers and bits */
   modbus_read_registers(mb_cli, 0, 1, &mb_register);
   ASSERT_VAL("read_and_verify_register_from_server(1)...", mb_register == 0xA5A5);

   modbus_read_registers(mb_cli, MB_MAP_REGISTERS-1, 1, &mb_register);
   ASSERT_VAL("read_and_verify_register_from_server(2)...", mb_register == 0xA5A5);

   modbus_read_bits(mb_cli, 0, 1, &mb_bit);
   ASSERT_VAL("read_and_verify_bits_from_server(1)...", mb_bit == 1);

   modbus_read_bits(mb_cli, MB_MAP_BITS-1, 1, &mb_bit);
   ASSERT_VAL("read_and_verify_bits_from_server(2)...", mb_bit == 1);
   modbus_free(mb_cli);


   PRINT_FOOTER();
}

static int test_stop_of_modbus_server(struct mb_server_data* mb_param) {
   int err_cntr = 0;
   int ok_cntr  = 0;
   int rc = 0;
   uint16_t mb_register = 0;
   modbus_t* mb_cli = NULL;

   PRINT_HEADER();

   mb_cli = modbus_new_tcp("127.0.0.1",TEST_PORT);
   modbus_connect(mb_cli);

   /* destroy context */
   rc = modbus_tcp_server_stop(mb_param->mb_srv_ctx);
   ASSERT_VAL("stop_server...", rc == 0);

   usleep(1000000);
   rc = modbus_read_registers(mb_cli, 1, 1, &mb_register);
   ASSERT_VAL("test_mb_read_fails_after_stop...", rc == -1);
   modbus_free(mb_cli);

   PRINT_FOOTER();
}

int main(void)
{
   int rc = 0;
   struct mb_server_data mb_param = {0};

   printf("\n");

   rc += test_start_modbus_server(&mb_param);
   rc += test_multiple_connections_to_modbus_server(&mb_param);
   rc += test_read_write_to_modbus_server();
   rc += test_stop_of_modbus_server(&mb_param);

   if(rc == 0) {
      printf("\n### ALL TESTS PASSED ###\n");
      return 0;
   }
   else {
      printf("\n\n### ONE OR MORE TESTS FAILED ### \n");
      return -1;
   }
}
