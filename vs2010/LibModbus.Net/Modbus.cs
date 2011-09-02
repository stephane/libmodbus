/*
 * Wraps the libmodbus functionality.
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
using System.Globalization;
using System.Runtime.InteropServices;
using System.Text;

namespace LibModbus
{
    /// <summary>
    /// Wraps the libmodbus functionality.
    /// </summary>
    public class Modbus : IDisposable
    {
        private bool disposed = false;
        internal ModbusHandle mb = null;

        public static readonly int False = 0;
        public static readonly int True = 1;
        public static readonly int Off = 0;
        public static readonly int On = 1;

        public static readonly int BroadcastAddress = 0;

        public static readonly int MaxReadBits = 2000;
        public static readonly int MaxWriteBits = 1968;

        public static readonly int MaxReadRegisters = 125;
        public static readonly int MaxWriteRegisters = 123;
        public static readonly int MaxRWWriteRegisters = 121;

        /* It's not really the minimal length (the real one is report slave ID
         * in RTU (4 bytes)) but it's a convenient size to use in RTU or TCP
         * communications to read many values or write a single one.
         * Maximum between :
         * - HEADER_LENGTH_TCP (7) + function (1) + address (2) + number (2)
         * - HEADER_LENGTH_RTU (1) + function (1) + address (2) + number (2) + CRC (2)
         */
        private const int _MinReqLength = 12;


        /// <summary>
        /// Enables messages for debugging, e.g. communication is written to stderr.
        /// </summary>
        private bool debug = false;
        public bool Debug
        {
            get { return debug; }
            set
            {
                debug = value;
                NativeMethods.modbus_set_debug(mb, Convert.ToInt32(debug));
            }
        }

        /// <summary>
        /// Sets the error recovery mode of the libmodbus driver.
        /// </summary>
        private ErrorRecoveryModes errorRecoveryMode = ErrorRecoveryModes.None;
        public ErrorRecoveryModes ErrorRecovery
        {
            get { return errorRecoveryMode; }
            set
            {
                if (NativeMethods.modbus_set_error_recovery(mb, value) == -1)
                {
                    throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                        "Could not set ErrorRecoveryMode to {1}: {0}.", GetLastError(), value));
                }
                errorRecoveryMode = value;
            }
        }

        /// <summary>
        /// Is true after connection is established.
        /// </summary>
        public bool IsConnected { get; private set; }

        /// <summary>
        /// In TCP mode: Establishes a modbus TCP connection with a Modbus server.
        /// In RTU mode: Sets up a serial port for RTU communications.
        /// </summary>
        public void Connect()
        {
            if (NativeMethods.modbus_connect(mb) == -1)
            {
                throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                    "Connection failed: {0}.", GetLastError()));
            }
            IsConnected = true;
        }

        /// <summary>
        /// In TCP mode: Closes the network connection and socket.
        /// In RTU mode: Closes the file descriptor.
        /// </summary>
        public void Close()
        {
            if (IsConnected)
            {
                if (!mb.IsInvalid)
                {
                    NativeMethods.modbus_close(mb);
                }
                IsConnected = false;
            }
        }

        /// <summary>
        /// Define the slave number.
        /// </summary>
        /// <param name="slaveid">ID of the slave</param>
        public void SetSlave(int slaveid)
        {
            if (NativeMethods.modbus_set_slave(mb, slaveid) == -1)
            {
                throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                    "Could not set the slave number to {1}: {0}.", GetLastError(), slaveid));
            }
        }

        /// <summary>
        /// Get/Set the timeout interval used to wait for a response.
        /// </summary>
        public TimeSpan ResponseTimeout
        {
            get
            {
                NativeMethods.TimevalStruct timeout = new NativeMethods.TimevalStruct();
                NativeMethods.modbus_get_response_timeout(mb, ref timeout);
                TimeSpan time = TimeSpan.FromSeconds(timeout.tv_sec) + TimeSpan.FromMilliseconds(timeout.tv_usec / 1000);
                return time;
            }
            set
            {
                NativeMethods.TimevalStruct timeout = new NativeMethods.TimevalStruct();
                timeout.tv_sec = Convert.ToInt32(value.TotalSeconds);
                timeout.tv_usec = Convert.ToInt32(value.TotalMilliseconds * 1000);
                NativeMethods.modbus_set_response_timeout(mb, ref timeout);
            }
        }

        /// <summary>
        /// Get/SEt the timeout interval between two consecutive bytes of a message.
        /// </summary>
        public TimeSpan ByteTimeout
        {
            get
            {
                NativeMethods.TimevalStruct timeout = new NativeMethods.TimevalStruct();
                NativeMethods.modbus_get_byte_timeout(mb, ref timeout);
                TimeSpan time = TimeSpan.FromSeconds(timeout.tv_sec) + TimeSpan.FromMilliseconds(timeout.tv_usec / 1000);
                return time;
            }
            set
            {
                NativeMethods.TimevalStruct timeout = new NativeMethods.TimevalStruct();
                timeout.tv_sec = Convert.ToInt32(value.TotalSeconds);
                timeout.tv_usec = Convert.ToInt32(value.TotalMilliseconds * 1000);
                NativeMethods.modbus_set_byte_timeout(mb, ref timeout);
            }
        }

        /// <summary>
        /// Turns ON or OFF a single bit of the remote device
        /// </summary>
        public void WriteBit(int coil_addr, int status)
        {
            if (NativeMethods.modbus_write_bit(mb, coil_addr, status) == -1)
            {
                throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                    "WriteBit (addr: {1}/0x{1:x}, status: {2}/0x{2:x})  failed: {0}.",
                    GetLastError(), coil_addr, status));
            }
        }

        /// <summary>
        /// Write the bits of the array in the remote device
        /// </summary>
        public void WriteBits(int addr, int nb, byte[] data)
        {
            if (null == data) { throw new ModbusException("Parameter data is not initialized."); }
            int size = Marshal.SizeOf(data[0]) * data.Length;
            IntPtr unmanagedPointer = Marshal.AllocHGlobal(size);
            try
            {
                Marshal.Copy(data, 0, unmanagedPointer, data.Length);
                if (NativeMethods.modbus_write_bits(mb, addr, nb, unmanagedPointer) == -1)
                {
                    throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                        "WriteBits (addr: {1}/0x{1:x}, nb: {2}/0x{2:x}, data[0]:{3}/0x{3:x}) failed: {0}.",
                        GetLastError(), addr, nb, data[0]));
                }
            }
            finally
            {
                Marshal.FreeHGlobal(unmanagedPointer);
            }
        }

        /// <summary>
        /// Reads the boolean status of bits and sets the array elements
        /// in the destination to TRUE or FALSE (single bits). 
        /// </summary>
        public void ReadBits(int addr, int nb, ref byte[] dest)
        {
            if (null == dest) { throw new ModbusException("Parameter dest is not initialized."); }
            int size = Marshal.SizeOf(dest[0]) * nb;
            IntPtr unmanagedPointer = Marshal.AllocHGlobal(size);
            try
            {
                if (NativeMethods.modbus_read_bits(mb, addr, nb, unmanagedPointer) == -1)
                {
                    throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                        "ReadBits (addr: {1}/0x{1:x}, nb: {2}/0x{2:x}) failed: {0}.", GetLastError(), addr, nb));
                }
                Marshal.Copy(unmanagedPointer, dest, 0, nb);
            }
            finally
            {
                Marshal.FreeHGlobal(unmanagedPointer);
            }
        }

        /// <summary>
        /// Same as modbus_read_bits but reads the remote device input table
        /// </summary>
        public void ReadInputBits(int addr, int nb, ref byte[] dest)
        {
            if (null == dest) { throw new ModbusException("Parameter dest is not initialized."); }
            int size = Marshal.SizeOf(dest[0]) * nb;
            IntPtr unmanagedPointer = Marshal.AllocHGlobal(size);
            try
            {
                if (NativeMethods.modbus_read_input_bits(mb, addr, nb, unmanagedPointer) == -1)
                {
                    throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                        "ReadInputBits (addr: {1}/0x{1:x}, nb: {2}/0x{2:x}) failed: {0}.",
                        GetLastError(), addr, nb));
                }
                Marshal.Copy(unmanagedPointer, dest, 0, nb);
            }
            finally
            {
                Marshal.FreeHGlobal(unmanagedPointer);
            }
        }

        /// <summary>
        /// Writes a value in one register of the remote device
        /// </summary>
        public void WriteRegister(int addr, int value)
        {
            if (NativeMethods.modbus_write_register(mb, addr, value) == -1)
            {
                throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                    "WriteRegister (addr: {1}/0x{1:x}, value: {2}/0x{2:x})  failed: {0}.",
                    GetLastError(), addr, value));
            }

        }

        /// <summary>
        ///  Write the values from the array to the registers of the remote device
        /// </summary>
        public void WriteRegisters(int addr, int nb, ushort[] data)
        {
            if (null == data) { throw new ModbusException("Parameter data is not initialized."); }
            int size = Marshal.SizeOf(data[0]) * data.Length;
            IntPtr unmanagedPointer = Marshal.AllocHGlobal(size);
            try
            {
                short[] tmp = new short[data.Length]; // Marshal.Copy does not operate on ushort but on short only
                System.Buffer.BlockCopy(data, 0, tmp, 0, size);
                Marshal.Copy(tmp, 0, unmanagedPointer, data.Length);
                if (NativeMethods.modbus_write_registers(mb, addr, nb, unmanagedPointer) == -1)
                {
                    throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                        "WriteRegisters (addr: {1}/0x{1:x}, nb: {2}/0x{2:x}, data[0]:{3}/0x{3:x}) failed: {0}.",
                        GetLastError(), addr, nb, data[0]));
                }
            }
            finally
            {
                Marshal.FreeHGlobal(unmanagedPointer);
            }
        }

        /// <summary>
        /// Reads the holding registers of remote device and put the data into an array.
        /// </summary>
        public void ReadRegisters(int addr, int nb, ref ushort[] dest)
        {
            if (null == dest) { throw new ModbusException("Parameter dest is not initialized."); }
            int size = Marshal.SizeOf(dest[0]) * nb;
            IntPtr unmanagedPointer = Marshal.AllocHGlobal(size);
            try
            {
                int ret = NativeMethods.modbus_read_registers(mb, addr, nb, unmanagedPointer) ;
                if (ret == -1)
                {
                    throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                        "ReadRegisters (addr: {1}/0x{1:x}, nb: {2}/0x{2:x}) failed: {0}.",
                        GetLastError(), addr, nb));
                }
                if (ret != nb)
                {
                    throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                        "ReadRegisters (addr: {1}/0x{1:x}, nb: {2}/0x{2:x}) returned only {0}/0x{0:x} instead of nb.",
                       ret, addr, nb));
                }
                short[] tmp = new short[nb]; // Marshal.Copy does not operate on ushort but on short only
                Marshal.Copy(unmanagedPointer, tmp, 0, nb);
                System.Buffer.BlockCopy(tmp, 0, dest, 0, size);
            }
            finally
            {
                Marshal.FreeHGlobal(unmanagedPointer);
            }
        }

        /// <summary>
        /// Reads the input registers of remote device and put the data into an array.
        /// </summary>
        public void ReadInputRegisters(int addr, int nb, ref ushort[] dest)
        {
            if (null == dest) { throw new ModbusException("Parameter dest is not initialized."); }
            int size = Marshal.SizeOf(dest[0]) * nb;
            IntPtr unmanagedPointer = Marshal.AllocHGlobal(size);
            try
            {
                if (NativeMethods.modbus_read_input_registers(mb, addr, nb, unmanagedPointer) == -1)
                {
                    throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                        "ReadInputRegisters (addr: {1}/0x{1:x}, nb: {2}/0x{2:x}) failed: {0}.",
                        GetLastError(), addr, nb));
                }
                short[] tmp = new short[nb]; // Marshal.Copy does not operate on ushort but on short only
                Marshal.Copy(unmanagedPointer, tmp, 0, nb);
                System.Buffer.BlockCopy(tmp, 0, dest, 0, size);
            }
            finally
            {
                Marshal.FreeHGlobal(unmanagedPointer);
            }
        }

        /// <summary>
        /// Write multiple registers from src array to remote device and read multiple
        /// registers from remote device to dest array.
        /// </summary>
        public void WriteAndReadRegisters(int write_addr, int write_nb, ushort[] src,
            int read_addr, int read_nb, ref ushort[] dest)
        {
            if (null == src) { throw new ModbusException("Parameter src is not initialized."); }
            if (null == dest) { throw new ModbusException("Parameter dest is not initialized."); }
            int sizeWrite = Marshal.SizeOf(src[0]) * src.Length;
            IntPtr unmanagedPointerData = Marshal.AllocHGlobal(sizeWrite);
            int sizeRead = Marshal.SizeOf(dest[0]) * read_nb;
            IntPtr unmanagedPointerDest = Marshal.AllocHGlobal(sizeRead);
            try
            {
                {
                    short[] tmp = new short[src.Length]; // Marshal.Copy does not operate on ushort but on short only
                    System.Buffer.BlockCopy(src, 0, tmp, 0, sizeWrite);
                    Marshal.Copy(tmp, 0, unmanagedPointerData, src.Length);
                }
                if (NativeMethods.modbus_write_and_read_registers(mb,
                    write_addr, write_nb, unmanagedPointerData,
                    read_addr, read_nb, unmanagedPointerDest) == -1)
                {
                    throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                        "WriteAndReadRegisters (write_addr: {1}/0x{1:x}, write_nb: {2}/0x{2:x}, data[0]:{3}/0x{3:x} - " +
                        "read_addr: {4}/0x{4:x}, read_nb: {5}/0x{5:x}) failed: {0}.",
                        GetLastError(), write_addr, write_nb, src[0], read_addr, read_nb));
                }
                {
                    short[] tmp = new short[read_nb];
                    Marshal.Copy(unmanagedPointerDest, tmp, 0, read_nb);
                    System.Buffer.BlockCopy(tmp, 0, dest, 0, sizeRead);
                }
            }
            finally
            {
                Marshal.FreeHGlobal(unmanagedPointerData);
                Marshal.FreeHGlobal(unmanagedPointerDest);
            }
        }

        ///<summary>
        /// Send a request to get the slave ID of the device (only available in serial
        /// communication).
        /// </summary>
        public int ReportSlaveId(ref byte[] dest)
        {
            if (null == dest) { throw new ModbusException("Parameter dest is not initialized."); }
            int size = Marshal.SizeOf(dest[0]) * _MinReqLength;
            IntPtr unmanagedPointer = Marshal.AllocHGlobal(size);
            try
            {
                int rc = NativeMethods.modbus_report_slave_id(mb, unmanagedPointer);
                if (rc <= 0)
                {
                    throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                        "ReportSlaveId failed (rc: {1}): {0}.",                        
                        GetLastError(), rc));
                }
                short[] tmp = new short[rc]; // Marshal.Copy does not operate on ushort but on short only
                Marshal.Copy(unmanagedPointer, tmp, 0, rc);
                System.Buffer.BlockCopy(tmp, 0, dest, 0, rc);
                return rc;
            }
            finally
            {
                Marshal.FreeHGlobal(unmanagedPointer);
            }
        }


        /// <summary>
        /// Get a float from 4 bytes in Modbus format.
        /// </summary>
        /// <param name="data">4 bytes in Modbus format</param>
        /// <returns>float</returns>
        public static float GetFloat(ushort[] data)
        {
            if (null == data) { throw new ModbusException("Parameter data is not initialized."); }
            float ret = float.NaN;
            int size = Marshal.SizeOf(data[0]) * data.Length;
            IntPtr unmanagedPointer = Marshal.AllocHGlobal(size);
            try
            {
                short[] tmp = new short[data.Length];
                System.Buffer.BlockCopy(data, 0, tmp, 0, size);
                Marshal.Copy(tmp, 0, unmanagedPointer, data.Length);
                ret = NativeMethods.modbus_get_float(unmanagedPointer);
            }
            finally
            {
                Marshal.FreeHGlobal(unmanagedPointer);
            }
            return ret;
        }

        /// <summary>
        /// Set a float to 4 bytes in Modbus format.
        /// </summary>
        /// <param name="value">float value</param>
        /// <param name="dest">value in Modbus format</param>
        public static void SetFloat(float value, ushort[] dest)
        {
            if (null == dest) { throw new ModbusException("Parameter dest is not initialized."); }
            int size = Marshal.SizeOf(dest[0]) * dest.Length;
            IntPtr unmanagedPointer = Marshal.AllocHGlobal(size);
            try
            {
                NativeMethods.modbus_set_float(value, unmanagedPointer);
                short[] tmp = new short[size];
                Marshal.Copy(unmanagedPointer, tmp, 0, size);
                System.Buffer.BlockCopy(tmp, 0, dest, 0, size);
            }
            finally
            {
                Marshal.FreeHGlobal(unmanagedPointer);
            }

        }

        public int SendRawRequest(byte[] raw_req, int raw_req_length)
        {
        ////DLL int modbus_send_raw_request(ModbusHandle ctx, uint8_t *raw_req, int raw_req_length);
        //[DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        //int modbus_send_raw_request(ModbusHandle ctx, IntPtr raw_req, int raw_req_length);
            if (null == raw_req) { throw new ModbusException("Parameter raw_req is not initialized."); }
            int size = Marshal.SizeOf(raw_req[0]) * raw_req_length;
            IntPtr unmanagedPointer = Marshal.AllocHGlobal(size);
            int ret = -1;
            try
            {
                Marshal.Copy(raw_req, 0, unmanagedPointer, raw_req_length);
                ret = NativeMethods.modbus_send_raw_request(mb, unmanagedPointer, raw_req_length);
                if (-1 == ret)
                {
                    throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                        "SendRawRequest (raw_req[0]: {1}/0x{1:x}, raw_req_length: {2}/0x{2:x}) failed: {0}.",
                        GetLastError(), raw_req[0], raw_req_length));
                }
            }
            finally
            {
                Marshal.FreeHGlobal(unmanagedPointer);
            }
            return ret;
        }

        /// <summary>
        /// Receives the confirmation.
        ///
        /// The function shall store the read response in rsp and return the number of
        /// values (bits or words). Otherwise, its shall return -1 and errno is set.
        /// 
        /// The function doesn't check the confirmation is the expected response to the
        /// initial request.
        /// </summary>
        public int ReceiveConfirmation(ref byte[] rsp)
        {
        ////DLL int modbus_receive_confirmation(modbus_t *ctx, uint8_t *rsp);
        //[DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        //int modbus_receive_confirmation(ModbusHandle ctx, IntPtr rsp);
            if (null == rsp) { throw new ModbusException("Parameter rsp is not initialized."); }
            int size = Marshal.SizeOf(rsp[0]) * rsp.Length;
            IntPtr unmanagedPointer = Marshal.AllocHGlobal(size);
             int ret = -1;
            try
            {
                ret = NativeMethods.modbus_receive_confirmation(mb, unmanagedPointer);
                if (-1 == ret)
                {
                    throw new ModbusException(string.Format(CultureInfo.CurrentCulture,
                        "ReceiveConfirmation failed: {0}.", GetLastError()));
                }
                Marshal.Copy(unmanagedPointer, rsp, 0,  rsp.Length);
            }
            finally
            {
                Marshal.FreeHGlobal(unmanagedPointer);
            }
            return ret;
        }


        /// <summary>
        /// Returns the last errno value from the DLL.
        /// </summary>
        public static string GetLastError()
        {
            StringBuilder sb = new StringBuilder(512);
            NativeMethods.modbus_last_error(sb, sb.Capacity + 1);
            return sb.ToString();
        }


        #region IDisposable Members incl. Finalizer

        ~Modbus()
        {
            Dispose(false);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                Close();
                if ((null != mb) && !mb.IsInvalid)
                {
                    mb.Dispose();
                }
                disposed = true;
            }
        }
        #endregion

        #region Modbus Utilities

        /// <summary>
        ///  Gets the byte value from many bits. To obtain a full byte, set nb_bits to 8.
        /// </summary>
        public static int GetByteFromBits(byte[] src, int index, uint nb_bits)
        {
            if (null == src) { throw new ModbusException("Parameter src is not initialized."); }
            int size = Marshal.SizeOf(src[0]) * src.Length;
            IntPtr unmanagedPointer = Marshal.AllocHGlobal(size);
            try
            {
                Marshal.Copy(src, 0, unmanagedPointer, src.Length);
                var ret = NativeMethods.modbus_get_byte_from_bits(unmanagedPointer, index, nb_bits);
                return ret;
            }
            finally
            {
                Marshal.FreeHGlobal(unmanagedPointer);
            }
        }

        /// <summary>
        /// Sets many bits from a table of bytes (only the bits between index and index + nb_bits are set)
        /// </summary>
        public static void SetBitsFromBytes(byte[] dest, int index, uint nb_bits, byte[] src)
        {
            if (null == dest) { throw new ModbusException("Parameter dest is not initialized."); }
            if (null == src) { throw new ModbusException("Parameter src is not initialized."); }
            int sizeSrc = Marshal.SizeOf(src[0]) * src.Length;
            int sizeDest = Marshal.SizeOf(dest[0]) * dest.Length;
            IntPtr unmanagedPointerSrc = Marshal.AllocHGlobal(sizeSrc);
            IntPtr unmanagedPointerDest = Marshal.AllocHGlobal(sizeDest);
            try
            {
                Marshal.Copy(src, 0, unmanagedPointerSrc, src.Length);
                NativeMethods.modbus_set_bits_from_bytes(unmanagedPointerDest, index, nb_bits, unmanagedPointerSrc);
                Marshal.Copy(unmanagedPointerDest, dest, 0, (int)nb_bits);

            }
            finally
            {
                Marshal.FreeHGlobal(unmanagedPointerSrc);
                Marshal.FreeHGlobal(unmanagedPointerDest);
            }
        }
        #endregion Modbus Utilities

    }
}
