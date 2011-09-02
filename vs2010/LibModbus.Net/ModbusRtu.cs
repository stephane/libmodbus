/*
 * Use Modbus RTU communcation.
 * Copyright © 2011 Christian Leutloff <leutloff@sundancer.oche.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
using System;

namespace LibModbus
{
    public class ModbusRtu : Modbus
    {
        public ModbusRtu(string device,
                         int baud,
                         char parity, int dataBit, int stopBit)
        {
            mb = NativeMethods.modbus_new_rtu(device, baud, parity, dataBit, stopBit);
            if (mb.IsInvalid)
            {
                throw new ModbusException("Unable to allocate libmodbus context for RTU operation.");
            }
        }
    }
}
