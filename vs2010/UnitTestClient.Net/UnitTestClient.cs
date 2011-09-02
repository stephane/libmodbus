/*
 * Copyright © 2011 Christian Leutloff <leutloff@sundancer.oche.de>
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
/*
This is the unit-test-client.c from Stéphane ported to C#.
The sequence of the tests and the coverage is the same, but the style is adapted to C#.
This belongs mostly to the Interface, e.g. in error cases exceptions are thrown.
The unit test itself is not changed without need.
*/

using System;
using System.Globalization;

namespace LibModbus
{
    class UnitTestClient
    {
        const int SERVER_ID = 17;
        const int INVALID_SERVER_ID = 18;


        const int UT_BITS_ADDRESS = 0x13;
        const int UT_BITS_NB = 0x25;

        const ushort UT_INPUT_BITS_ADDRESS = 0xC4;
        const ushort UT_INPUT_BITS_NB = 0x16;

        const ushort UT_REGISTERS_ADDRESS = 0x6B;
        /* Raise a manual exception when this adress is used for the first byte */
        const ushort UT_REGISTERS_ADDRESS_SPECIAL = 0x6C;
        const ushort UT_REGISTERS_NB = 0x3;
        /* If the following value is used, a bad response is sent.
           It's better to test with a lower value than
           UT_REGISTERS_NB_POINTS to try to raise a segfault. */
        const ushort UT_REGISTERS_NB_SPECIAL = 0x2;

        const ushort UT_INPUT_REGISTERS_ADDRESS = 0x08;
        const ushort UT_INPUT_REGISTERS_NB = 0x1;

        const float UT_REAL = 916.540649f;
        const UInt32 UT_IREAL = 0x4465229a;


        static void Main()
        {
            byte[] UT_BITS_TAB = { 0xCD, 0x6B, 0xB2, 0x0E, 0x1B };
            byte[] UT_INPUT_BITS_TAB = { 0xAC, 0xDB, 0x35 };
            ushort[] UT_REGISTERS_TAB = { 0x022B, 0x0001, 0x0064 };
            ushort[] UT_INPUT_REGISTERS_TAB = { 0x000A };

            System.Console.WriteLine("Unit Test Client using Modbus TCP");
            try
            {
                //Modbus ctx = new ModbusRtu(SERIALPORT_DEVICE_NAME, SERIALPORT_BAUD,
                //    SERIALPORT_PARITY, SERIALPORT_DATA_BITS, SERIALPORT_STOP_BIT);
                using (Modbus ctx = new ModbusTcp("127.0.0.1", 1502))
                {
                    ctx.Debug = true;
                    ctx.ErrorRecovery = ErrorRecoveryModes.LinkAndProtocol;

                    ctx.Connect();

                    Console.Write("** UNIT TESTING **\n");

                    Console.Write("\nTEST WRITE/READ:\n");

                    /** COIL BITS **/

                    /* Single */
                    Console.Write("1/2 modbus_write_bit: ");
                    ctx.WriteBit(UT_BITS_ADDRESS, Modbus.On);
                    uint nb_points = (UT_BITS_NB > UT_INPUT_BITS_NB) ? UT_BITS_NB : UT_INPUT_BITS_NB;
                    byte[] tab_rp_bits = new byte[nb_points];
                    Console.Write("2/2 modbus_read_bits: ");
                    ctx.ReadBits(UT_BITS_ADDRESS, 1, ref tab_rp_bits);
                    if (tab_rp_bits[0] != Modbus.On)
                    {
                        Console.Write(string.Format(CultureInfo.CurrentCulture, "FAILED ({0:X} != {1:X})\n", tab_rp_bits[0], Modbus.On));
                        throw new ApplicationException("ReadBits failed");
                    }
                    else { Console.Write("OK\n"); }
                    /* End single */


                    /* Multiple bits */
                    {
                        Console.Write("1/2 modbus_write_bits: ");
                        byte[] tab_value = new byte[UT_BITS_NB];
                        Modbus.SetBitsFromBytes(tab_value, 0, UT_BITS_NB, UT_BITS_TAB);
                        ctx.WriteBits(UT_BITS_ADDRESS, UT_BITS_NB, tab_value);
                    }

                    Console.Write("2/2 modbus_read_bits: ");
                    ctx.ReadBits(UT_BITS_ADDRESS, UT_BITS_NB, ref tab_rp_bits);
                    {
                        int i = 0;
                        nb_points = UT_BITS_NB;
                        while (nb_points > 0)
                        {
                            uint nb_bits = (nb_points > 8) ? 8 : nb_points;
                            var value = Modbus.GetByteFromBits(tab_rp_bits, i * 8, nb_bits);
                            if (value != UT_BITS_TAB[i])
                            {
                                Console.Write(string.Format(CultureInfo.CurrentCulture, "FAILED (0x{0:x} != 0x{1:x}, {2: 8} != {3:8})\n", value, UT_BITS_TAB[i], Convert.ToString(value, 2), Convert.ToString(UT_BITS_TAB[i], 2)));
                                throw new ApplicationException("ReadBits failed");
                            }
                            nb_points -= nb_bits;
                            i++;
                        }
                        Console.Write("OK\n");
                    }
                    /* End of multiple bits */

                    /** DISCRETE INPUTS **/
                    Console.Write("1/1 modbus_read_input_bits: ");
                    ctx.ReadInputBits(UT_INPUT_BITS_ADDRESS, UT_INPUT_BITS_NB, ref tab_rp_bits);
                    {
                        int i = 0;
                        nb_points = UT_INPUT_BITS_NB;
                        while (nb_points > 0)
                        {
                            uint nb_bits = (nb_points > 8) ? 8 : nb_points;
                            int value = Modbus.GetByteFromBits(tab_rp_bits, i * 8, nb_bits);
                            if (value != UT_INPUT_BITS_TAB[i])
                            {
                                Console.Write(string.Format(CultureInfo.CurrentCulture, "FAILED ({0:X} != {1:X})\n", value, UT_INPUT_BITS_TAB[i]));
                                throw new ApplicationException(" failed");
                            }

                            nb_points -= nb_bits;
                            i++;
                        }
                        Console.Write("OK\n");
                    }
                    /** HOLDING REGISTERS **/

                    /* Single register */
                    nb_points = (UT_REGISTERS_NB > UT_INPUT_REGISTERS_NB) ? UT_REGISTERS_NB : UT_INPUT_REGISTERS_NB;
                    ushort[] tab_rp_registers = new ushort[nb_points];
                    Console.Write("1/2 modbus_write_register: ");
                    ctx.WriteRegister(UT_REGISTERS_ADDRESS, 0x1234);
                    Console.Write("OK\n");
                    Console.Write("2/2 modbus_read_registers: ");
                    ctx.ReadRegisters(UT_REGISTERS_ADDRESS, 1, ref tab_rp_registers);
                    if (tab_rp_registers[0] != 0x1234)
                    {
                        Console.Write(string.Format(CultureInfo.CurrentCulture, "FAILED ({0:X} != {1:X})\n",
                               tab_rp_registers[0], 0x1234));
                        throw new ApplicationException("ReadRegisters (Single register) failed");
                    }
                    Console.Write("OK\n");
                    /* End of single register */

                    /* Many registers */
                    Console.Write("1/5 modbus_write_registers: ");
                    ctx.WriteRegisters(UT_REGISTERS_ADDRESS, UT_REGISTERS_NB, UT_REGISTERS_TAB);
                    Console.Write("OK\n");

                    Console.Write("2/5 modbus_read_registers: ");
                    ctx.ReadRegisters(UT_REGISTERS_ADDRESS, UT_REGISTERS_NB, ref tab_rp_registers);

                    for (int i = 0; i < UT_REGISTERS_NB; i++)
                    {
                        if (tab_rp_registers[i] != UT_REGISTERS_TAB[i])
                        {
                            Console.Write(string.Format(CultureInfo.CurrentCulture, "FAILED ({0:X} != {1:X})\n",
                                   tab_rp_registers[i],
                                   UT_REGISTERS_TAB[i]));
                            throw new ApplicationException("ReadRegisters failed");
                        }
                    }
                    Console.Write("OK\n");

                    Console.Write("3/5 modbus_read_registers (0): ");
                    ctx.ReadRegisters(UT_REGISTERS_ADDRESS, 0, ref tab_rp_registers);
                    Console.Write("OK\n");

                    /* Write registers to zero from tab_rp_registers and store read registers
                       into tab_rp_registers. So the read registers must set to 0, except the
                       first one because there is an offset of 1 register on write. */
                    Console.Write("4/5 modbus_write_and_read_registers: ");
                    {
                        for (int i = 0; i < tab_rp_registers.Length; ++i) { tab_rp_registers[i] = 0; } // clear tab_rp_registers
                        ctx.WriteAndReadRegisters(UT_REGISTERS_ADDRESS + 1, UT_REGISTERS_NB - 1, tab_rp_registers,
                                                  UT_REGISTERS_ADDRESS, UT_REGISTERS_NB, ref  tab_rp_registers);
                        if (tab_rp_registers[0] != UT_REGISTERS_TAB[0])
                        {
                            Console.Write(string.Format(CultureInfo.CurrentCulture, "FAILED 0 ({0:X} != {1:X})\n",
                                   tab_rp_registers[0], UT_REGISTERS_TAB[0]));
                        }

                        for (int i = 1; i < UT_REGISTERS_NB; i++)
                        {
                            if (tab_rp_registers[i] != 0)
                            {
                                Console.Write(string.Format(CultureInfo.CurrentCulture, "FAILED i=={2}: ({0:X} != {1:X})\n",
                                       tab_rp_registers[i], 0, i));
                                throw new ApplicationException("WriteAndReadRegisters failed");
                            }
                        }
                    }
                    Console.Write("OK\n");
                    /* End of many registers */

                    /** INPUT REGISTERS **/
                    Console.Write("1/1 modbus_read_input_registers: ");
                    ctx.ReadInputRegisters(UT_INPUT_REGISTERS_ADDRESS, UT_INPUT_REGISTERS_NB, ref tab_rp_registers);
                    for (int i = 0; i < UT_INPUT_REGISTERS_NB; i++)
                    {
                        if (tab_rp_registers[i] != UT_INPUT_REGISTERS_TAB[i])
                        {
                            Console.Write(string.Format(CultureInfo.CurrentCulture, "FAILED ({0:X} != {1:X})\n",
                                   tab_rp_registers[i], UT_INPUT_REGISTERS_TAB[i]));
                            throw new ApplicationException("ReadInputRegisters failed");
                        }
                    }
                    Console.Write("OK\n");

                    Console.Write("\nTEST FLOATS\n");
                    /** FLOAT **/
                    Console.Write("1/2 Set float: ");
                    Modbus.SetFloat(UT_REAL, tab_rp_registers);
                    if (tab_rp_registers[1] == (UT_IREAL >> 16) &&
                        tab_rp_registers[0] == (UT_IREAL & 0xFFFF))
                    {
                        Console.Write("OK\n");
                    }
                    else
                    {
                        Console.Write(string.Format(CultureInfo.CurrentCulture, "FAILED ({0:X} != {1:X})\n",
                              tab_rp_registers[1] << 16 + tab_rp_registers[0], UT_IREAL));
                        throw new ApplicationException("SetFloat failed");
                    }

                    Console.Write("2/2 Get float: ");
                    float real = Modbus.GetFloat(tab_rp_registers);
                    if (real == UT_REAL)
                    {
                        Console.Write("OK\n");
                    }
                    else
                    {
                        Console.Write(string.Format(CultureInfo.CurrentCulture, "FAILED ({0:X} != {1:X})\n", real, UT_REAL));
                        throw new ApplicationException("GetFloat failed");
                    }

                    Console.Write("\n******************************************************************************");
                    Console.Write("\nAt this point, error messages doesn't mean the test has failed,\n");
                    Console.Write("because error conditions are tested.\n");

                    /** ILLEGAL DATA ADDRESS **/
                    Console.Write("\nTEST ILLEGAL DATA ADDRESS:\n");

                    /* The mapping begins at 0 and ends at address + nb_points so
                     * the addresses are not valid. */
                    bool IsExceptionCaught = false;
                    try
                    {
                        Console.Write("* Readbits: ");
                        ctx.ReadBits(UT_BITS_ADDRESS, UT_BITS_NB + 1, ref tab_rp_bits);
                    }
                    catch (ModbusException)//if (rc == -1 && errno == EMBXILADD)
                    {
                        Console.Write("OK\n");
                        IsExceptionCaught = true;
                    }
                    if (!IsExceptionCaught)
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException(" failed");
                    }

                    IsExceptionCaught = false;
                    try
                    {
                        Console.Write("* ReadInputbits: ");
                        ctx.ReadInputBits(UT_INPUT_BITS_ADDRESS, UT_INPUT_BITS_NB + 1, ref tab_rp_bits);
                    }
                    catch (ModbusException)//if (rc == -1 && errno == EMBXILADD)
                    {
                        Console.Write("OK\n");
                        IsExceptionCaught = true;
                    }
                    if (!IsExceptionCaught)
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException(" failed");
                    }

                    IsExceptionCaught = false;
                    try
                    {
                        Console.Write("* Readregisters: ");
                        ctx.ReadRegisters(UT_REGISTERS_ADDRESS, UT_REGISTERS_NB + 1, ref tab_rp_registers);
                    }
                    catch (ModbusException)// if (rc == -1 && errno == EMBXILADD)
                    {
                        Console.Write("OK\n");
                        IsExceptionCaught = true;
                    }
                    if (!IsExceptionCaught)
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException(" failed");
                    }

                    IsExceptionCaught = false;
                    try
                    {
                        Console.Write("* ReadInputregisters: ");
                        ctx.ReadInputRegisters(UT_INPUT_REGISTERS_ADDRESS, UT_INPUT_REGISTERS_NB + 1, ref tab_rp_registers);
                    }
                    catch (ModbusException)//if (rc == -1 && errno == EMBXILADD)
                    {
                        Console.Write("OK\n");
                        IsExceptionCaught = true;
                    }
                    if (!IsExceptionCaught)
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException(" failed");
                    }

                    IsExceptionCaught = false;
                    try
                    {
                        Console.Write("* Writebit: ");
                        ctx.WriteBit(UT_BITS_ADDRESS + UT_BITS_NB, Modbus.On);
                    }
                    catch (ModbusException)//  if (rc == -1 && errno == EMBXILADD)
                    {
                        Console.Write("OK\n");
                        IsExceptionCaught = true;
                    }
                    if (!IsExceptionCaught)
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException(" failed");
                    }

                    IsExceptionCaught = false;
                    try
                    {
                        Console.Write("* Writecoils: ");
                        ctx.WriteBits(UT_BITS_ADDRESS + UT_BITS_NB, UT_BITS_NB, tab_rp_bits);
                    }
                    catch (ModbusException)//  if (rc == -1 && errno == EMBXILADD)
                    {
                        Console.Write("OK\n");
                        IsExceptionCaught = true;
                    }
                    if (!IsExceptionCaught)
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException(" failed");
                    }

                    IsExceptionCaught = false;
                    try
                    {
                        Console.Write("* Writeregisters: ");
                        ctx.WriteRegisters(UT_REGISTERS_ADDRESS + UT_REGISTERS_NB, UT_REGISTERS_NB, tab_rp_registers);
                    }
                    catch (ModbusException)// if (rc == -1 && errno == EMBXILADD)
                    {
                        Console.Write("OK\n");
                        IsExceptionCaught = true;
                    }
                    if (!IsExceptionCaught)
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException(" failed");
                    }

                    /** TOO MANY DATA **/
                    Console.Write("\nTEST TOO MANY DATA ERROR:\n");

                    IsExceptionCaught = false;
                    try
                    {
                        Console.Write("* Readbits: ");
                        ctx.ReadBits(UT_BITS_ADDRESS, Modbus.MaxReadBits + 1, ref tab_rp_bits);
                    }
                    catch (ModbusException)//if (rc == -1 && errno == EMBMDATA)
                    {
                        Console.Write("OK\n");
                        IsExceptionCaught = true;
                    }
                    if (!IsExceptionCaught)
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException(" failed");
                    }

                    IsExceptionCaught = false;
                    try
                    {
                        Console.Write("* ReadInputbits: ");
                        ctx.ReadInputBits(UT_INPUT_BITS_ADDRESS, Modbus.MaxReadBits + 1, ref tab_rp_bits);
                    }
                    catch (ModbusException)//if (rc == -1 && errno == EMBMDATA)
                    {
                        Console.Write("OK\n");
                        IsExceptionCaught = true;
                    }
                    if (!IsExceptionCaught)
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException(" failed");
                    }

                    IsExceptionCaught = false;
                    try
                    {
                        Console.Write("* Readregisters: ");
                        ctx.ReadRegisters(UT_REGISTERS_ADDRESS, Modbus.MaxReadRegisters + 1, ref   tab_rp_registers);
                    }
                    catch (ModbusException)//if (rc == -1 && errno == EMBMDATA)
                    {
                        Console.Write("OK\n");
                        IsExceptionCaught = true;
                    }
                    if (!IsExceptionCaught)
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException(" failed");
                    }

                    IsExceptionCaught = false;
                    try
                    {
                        Console.Write("* ReadInputregisters: ");
                        ctx.ReadInputRegisters(UT_INPUT_REGISTERS_ADDRESS, Modbus.MaxReadRegisters + 1, ref  tab_rp_registers);
                    }
                    catch (ModbusException)//if (rc == -1 && errno == EMBMDATA)
                    {
                        Console.Write("OK\n");
                        IsExceptionCaught = true;
                    }
                    if (!IsExceptionCaught)
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException(" failed");
                    }

                    IsExceptionCaught = false;
                    try
                    {
                        Console.Write("* Writebits: ");
                        ctx.WriteBits(UT_BITS_ADDRESS, Modbus.MaxWriteBits + 1, tab_rp_bits);
                    }
                    catch (ModbusException)//if (rc == -1 && errno == EMBMDATA)
                    {
                        Console.Write("OK\n");
                        IsExceptionCaught = true;
                    }
                    if (!IsExceptionCaught)
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException(" failed");
                    }

                    IsExceptionCaught = false;
                    try
                    {
                        Console.Write("* Writeregisters: ");
                        ctx.WriteRegisters(UT_REGISTERS_ADDRESS, Modbus.MaxWriteRegisters + 1, tab_rp_registers);
                    }
                    catch (ModbusException)//if (rc == -1 && errno == EMBMDATA)
                    {
                        Console.Write("OK\n");
                        IsExceptionCaught = true;
                    }
                    if (!IsExceptionCaught)
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException(" failed");
                    }

                    /** SLAVE REPLY **/
                    Console.Write("\nTEST SLAVE REPLY:\n");
                    ctx.SetSlave(INVALID_SERVER_ID);
                    try
                    {
                        Console.Write(string.Format(CultureInfo.CurrentCulture, "1/4 Response from slave {0}: ", INVALID_SERVER_ID));
                        ctx.ReadRegisters(UT_REGISTERS_ADDRESS, UT_REGISTERS_NB, ref tab_rp_registers);
                        Console.Write("OK\n");
                    }
                    catch (ModbusException)//if (rc == UT_REGISTERS_NB) { // in TCP mode setting the slave id to a wrong value doesn't matter
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException(" failed");
                    }

                    try
                    {
                        ctx.SetSlave(Modbus.BroadcastAddress);
                    }
                    catch (ModbusException)//  if (rc == -1) 
                    {
                        Console.Write("Invalid broacast address, because it is not accepted as valid.\n");
                        Console.Write("FAILED\n");
                        throw new ApplicationException("SetSlave(Modbus.BroadcastAddress) failed");
                    }

                    try
                    {
                        Console.Write("2/4 Reply after a broadcast query: ");
                        ctx.ReadRegisters(UT_REGISTERS_ADDRESS,
                                 UT_REGISTERS_NB, ref tab_rp_registers);
                        Console.Write("OK\n");
                    }
                    catch (ModbusException)//if (rc == UT_REGISTERS_NB) {
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException("ReadRegisters failed");
                    }


                    /* Restore slave */
                    ctx.SetSlave(ModbusTcp.TcpSlave);

                    Console.Write("3/4 Report slave ID: \n");
                    /* tab_rp_bits is used to store bytes */
                    try
                    {
                        int rc = ctx.ReportSlaveId(ref tab_rp_bits);
                        /* Slave ID is an arbitraty number for libmodbus */
                        Console.Write(string.Format(CultureInfo.CurrentCulture, "OK Slave ID is {0}.\n", tab_rp_bits[0]));
                        /* Run status indicator */
                        if (rc > 1 && tab_rp_bits[1] == 0xFF)
                        {
                            Console.Write(string.Format(CultureInfo.CurrentCulture, "OK Run Status Indicator is {0}.\n", (Modbus.On == tab_rp_bits[1]) ? "ON" : "OFF"));
                        }
                        else
                        {
                            Console.Write("FAILED\n");
                            throw new ApplicationException(" failed");
                        }

                        /* Print additional data as string */
                        if (rc > 2)
                        {
                            Console.Write("Additional data: ");
                            for (int i = 2; i < rc; i++)
                            {
                                Console.Write(string.Format(CultureInfo.InvariantCulture, "{0}", tab_rp_bits[i]));
                            }
                            Console.Write("\n");
                        }
                    }
                    catch (ModbusException)//if (rc == -1) {
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException("ReportSlaveId failed");
                    }


                    /* Save original timeout */
                    TimeSpan oldResponseTimeout = ctx.ResponseTimeout;

                    /* Define a new and too short timeout */
                    // responseTimeout.tv_sec = 0; responseTimeout.tv_usec = 0;
                    ctx.ResponseTimeout = new TimeSpan(0);
                    IsExceptionCaught = false;
                    try
                    {
                        Console.Write("4/4 Too short timeout: ");
                        ctx.ReadRegisters(UT_REGISTERS_ADDRESS,
                               UT_REGISTERS_NB, ref tab_rp_registers);
                    }
                    catch (ModbusException)//if (rc == -1 && errno == ETIMEDOUT) 
                    {
                        Console.Write("OK\n");
                        IsExceptionCaught = true;
                    }
                    if (!IsExceptionCaught)
                    {
                        Console.Write("FAILED (can fail on slow systems or Windows).\n");
                        throw new ApplicationException(" failed");
                    }

                    /* Restore original timeout */
                    ctx.ResponseTimeout = oldResponseTimeout;

                    /* A wait and flush operation is done by the error recovery code of
                     * libmodbus */

                    /** BAD RESPONSE **/
                    Console.Write("\nTEST BAD RESPONSE ERROR:\n");

                    /* Allocate only the required space */
                    {
                        ushort[] tab_rp_registers_bad = new ushort[UT_REGISTERS_NB_SPECIAL];
                        IsExceptionCaught = false;
                        try
                        {
                            ctx.ReadRegisters(UT_REGISTERS_ADDRESS,
                                     UT_REGISTERS_NB_SPECIAL, ref tab_rp_registers_bad);
                            Console.Write("* Readregisters: ");
                        }
                        catch (ModbusException)// if (rc == -1 && errno == EMBBADDATA) {
                        {
                            Console.Write("OK\n");
                            IsExceptionCaught = true;
                        }
                        if (!IsExceptionCaught)
                        {
                            Console.Write("FAILED\n");
                            throw new ApplicationException(" failed");
                        }


                        tab_rp_registers_bad = null;
                    }

                    /** MANUAL EXCEPTION **/
                    Console.Write("\nTEST MANUAL EXCEPTION:\n");

                    IsExceptionCaught = false;
                    try
                    {
                        Console.Write("* Readregisters at special address: ");
                        ctx.ReadRegisters(UT_REGISTERS_ADDRESS_SPECIAL, UT_REGISTERS_NB, ref tab_rp_registers);
                    }
                    catch (ModbusException)//if (rc == -1 && errno == EMBXSBUSY) {
                    {
                        Console.Write("OK\n");
                        IsExceptionCaught = true;
                    }
                    if (!IsExceptionCaught)
                    {
                        Console.Write("FAILED\n");
                        throw new ApplicationException(" failed");
                    }

                    /** RAW REQUEST */
                    Console.Write("\nTEST RAW REQUEST:\n");
                    {
                        const int RAW_REQ_LENGTH = 6;
                        byte[] raw_req = { 0xFF, 0x03, 0x00, 0x01, 0x0, 0x05 };
                        int req_length;
                        byte[] rsp = new byte[ModbusTcp.TcpMaxAduLength];

                        Console.Write("* send_raw_request: ");
                        req_length = ctx.SendRawRequest(raw_req, RAW_REQ_LENGTH * sizeof(byte));

                        if (req_length == (RAW_REQ_LENGTH + 6))
                        {
                            Console.Write("OK\n");
                        }
                        else
                        {
                            Console.Write("FAILED\n");
                            throw new ApplicationException(" failed");
                        }


                        Console.Write("* receive_confirmation: ");
                        int rc = ctx.ReceiveConfirmation(ref rsp);
                        if (rc == 19)
                        {
                            Console.Write("OK\n");// if ((use_backend == RTU && rc == 15) || ((use_backend == TCP || use_backend == TCP_PI) && rc == 19)) {
                        }
                        else
                        {
                            Console.Write("FAILED\n");
                            throw new ApplicationException(" failed");
                        }
                    }
                    ctx.Close();
                    Console.Write("\nALL TESTS PASS WITH SUCCESS.\n");
                } // using ctx
            }
            catch (ModbusException ex)
            {
                Console.WriteLine(string.Format(CultureInfo.CurrentCulture, "Unit Test Client failed within Modbus part. Reason: {0}.", ex.Message));
                Console.WriteLine(ex.ToString());
                Console.WriteLine(string.Format(CultureInfo.CurrentCulture, "Reason \"Connection failed: No error\" often means that the server was not started."));
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
        }
    }
}
