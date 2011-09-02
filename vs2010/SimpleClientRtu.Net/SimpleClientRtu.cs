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
This is small client with hard coded values to read a single register
value from a Modbus RTU connected device. 
This is the same program as simpleclient-rtu.c, but ported to C#.

It was successfully used on a Win 7, SP1, 64 bit System. Though the 
executable still was a 32 bit one.
*/
using System;
using System.Globalization;

namespace LibModbus
{
    class SimpleClientRtu
    {
        // Serial interface 
        const string SERIALPORT_DEVICE_NAME = "COM11";
        const int SERIALPORT_BAUD = 38400;
        const char SERIALPORT_PARITY = 'O'; // one of 'N', 'O', 'E'
        const int SERIALPORT_DATA_BITS = 8;
        const int SERIALPORT_STOP_BIT = 1;

        // used device
        const int SLAVE_ID = 65; // modbus id
        const int READ_REGISTER_ADDRESS = 2056;
        const int READ_REGISTER_EXPECTED_VALUE = 1635;
        const UInt16 UT_INPUT_REGISTERS_NB = 0x1;

        static void Main()
        {
            System.Console.WriteLine("Simple Client for Modbus RTU");
            try
            {
                //Modbus ctx = new ModbusRtu(SERIALPORT_DEVICE_NAME, SERIALPORT_BAUD,
                //    SERIALPORT_PARITY, SERIALPORT_DATA_BITS, SERIALPORT_STOP_BIT);
                using (ModbusRtu ctx = new ModbusRtu(SERIALPORT_DEVICE_NAME, SERIALPORT_BAUD,
                    SERIALPORT_PARITY, SERIALPORT_DATA_BITS, SERIALPORT_STOP_BIT))
                {
                    ctx.Debug = true;
                    ctx.ErrorRecovery = ErrorRecoveryModes.LinkAndProtocol;

                    ctx.SetSlave(SLAVE_ID);
                    ctx.Connect();

                    // read single register (function 4)
                    int nb_points = UT_INPUT_REGISTERS_NB;
                    ushort[] tab_rp_registers = new ushort[nb_points];
                    Console.Write("Read Input Registers: ");
                    ctx.ReadInputRegisters(READ_REGISTER_ADDRESS, 1, ref tab_rp_registers);
                    if (tab_rp_registers[0] != READ_REGISTER_EXPECTED_VALUE)// the expected value to read is 1635
                    {
                        Console.Write(string.Format(CultureInfo.CurrentCulture, "FAILED ({0}/0x{0:x} != {1}/0x{1:x})\n",
                            tab_rp_registers[0], READ_REGISTER_EXPECTED_VALUE));
                        throw new ApplicationException("The expected value was not read.");
                    }
                    else
                    {
                        Console.Write(string.Format(CultureInfo.CurrentCulture, "received: 0x{0:x} ({0})\n",
                            tab_rp_registers[0]));
                    }
                    
                    // clean up
                    ctx.Close();
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
