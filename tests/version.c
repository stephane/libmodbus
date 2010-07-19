/*
 * Copyright © 2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <modbus.h>

int main(void)
{
    printf("Compiled with libmodbus version %s\n", LIBMODBUS_VERSION_STRING);
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
