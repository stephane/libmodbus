/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <modbus.h>

int main(void)
{
    printf("Compiled with libmodbus version %s (%06X)\n", LIBMODBUS_VERSION_STRING, LIBMODBUS_VERSION_HEX);
    printf("Linked with libmodbus version %d.%d.%d\n",
           libmodbus_version_major, libmodbus_version_minor, libmodbus_version_micro);

    if (LIBMODBUS_VERSION_CHECK(2, 1, 0)) {
        printf("The functions to read/write float values are available (2.1.0).\n");
    }

    if (LIBMODBUS_VERSION_CHECK(2, 1, 1)) {
        printf("Oh gosh, brand new API (2.1.1)!\n");
    }

    return 0;
}
