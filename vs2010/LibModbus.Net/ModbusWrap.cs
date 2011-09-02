/*
 * Make the functions of libmodbus.dll available to C#.
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
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Text;
using Microsoft.Win32.SafeHandles;

namespace LibModbus
{
    /// <summary>
    /// SafeHandle to avoid ressource leaking.
    /// </summary>
    [SecurityPermission(SecurityAction.InheritanceDemand, UnmanagedCode = true)]
    [SecurityPermission(SecurityAction.Demand, UnmanagedCode = true)]
    internal class ModbusHandle : SafeHandleZeroOrMinusOneIsInvalid
    {
        private ModbusHandle()
            : base(true)
        {
        }
        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
        override protected bool ReleaseHandle()
        {
            NativeMethods.modbus_free(handle);
            return true;
        }
    }

    /// <summary>
    /// A managed enum matching the modbus_error_recovery_mode enum in modbus.h.
    /// </summary>
    [Flags]
    public enum ErrorRecoveryModes : int
    {
        None = 0, //  MODBUS_ERROR_RECOVERY_NONE = 0,
        Link = (1 << 1), // MODBUS_ERROR_RECOVERY_LINK = (1 << 1),
        Protocol = (1 << 2),// MODBUS_ERROR_RECOVERY_PROTOCOL = (1 << 2),
        LinkAndProtocol = Link | Protocol //     MODBUS_ERROR_RECOVERY_LINK_AND_PROTOCOL = MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL
    }

    /// <summary>
    /// Wraps the modbus.h functions including modbus-rth.h and modbus-tcp.h.
    /// Focus is on the ones required by a client.
    /// </summary>
    internal static class NativeMethods
    {
        [StructLayout(LayoutKind.Sequential)]
        internal struct ModbusMapping
        {
            int nb_bits;
            int nb_input_bits;
            int nb_input_registers;
            int nb_registers;
            IntPtr tab_bits; //  uint8_t *
            IntPtr tab_input_bits; //  uint8_t *
            IntPtr tab_input_registers;//  uint16_t *
            IntPtr tab_registers;//  uint16_t *
        }

        /// <summary>
        /// This timeval struct is only internally used, when accesing the time out related functions.
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        internal struct TimevalStruct
        {
            public Int32 tv_sec;
            public Int32 tv_usec;
        }

        #region Modbus
        //DLL int modbus_set_slave(modbus_t* ctx, int slave);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_set_slave(ModbusHandle ctx, int slave);

        //DLL int modbus_set_error_recovery(modbus_t *ctx, modbus_error_recovery_mode error_recovery);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_set_error_recovery(ModbusHandle ctx, ErrorRecoveryModes error_recovery);

        // not useful to get the socket for C#!?
        //DLL void modbus_set_socket(modbus_t *ctx, int socket);
        //DLL int modbus_get_socket(modbus_t *ctx);

        //DLL void modbus_get_response_timeout(modbus_t *ctx, struct timeval *timeout);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void modbus_get_response_timeout(ModbusHandle ctx, ref TimevalStruct timeout);

        //DLL void modbus_set_response_timeout(modbus_t *ctx, const struct timeval *timeout);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void modbus_set_response_timeout(ModbusHandle ctx, ref TimevalStruct timeout);

        //DLL void modbus_get_byte_timeout(modbus_t *ctx, struct timeval *timeout);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void modbus_get_byte_timeout(ModbusHandle ctx, ref TimevalStruct timeout);
        //DLL void modbus_set_byte_timeout(modbus_t *ctx, const struct timeval *timeout);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void modbus_set_byte_timeout(ModbusHandle ctx, ref TimevalStruct timeout);

        //DLL int modbus_get_header_length(modbus_t *ctx);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_get_header_length(ModbusHandle ctx);

        //DLL int modbus_connect(modbus_t *ctx);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_connect(ModbusHandle ctx);
        //DLL void modbus_close(modbus_t *ctx);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void modbus_close(ModbusHandle ctx);

        //DLL void modbus_free(modbus_t *ctx);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void modbus_free(IntPtr ctx);

        //DLL int modbus_flush(modbus_t *ctx);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_flush(ModbusHandle ctx);

        //DLL void modbus_set_debug(modbus_t *ctx, int boolean);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void modbus_set_debug(ModbusHandle ctx, int boolean);

        //DLL const char *modbus_strerror(int errnum);
        // the following definition throws exceptions "Memory Access Violation" when I used it 8-(
        //[DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi, BestFitMapping = false, ThrowOnUnmappableChar = true)]
        //[return: MarshalAs(UnmanagedType.LPStr)]
        //public static extern StringBuilder modbus_strerror(int errnum);

        //DLL void modbus_last_error(char *err_msg, int max_msg_size);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Ansi, BestFitMapping = false, ThrowOnUnmappableChar = true)]
        public static extern void modbus_last_error([MarshalAs(UnmanagedType.LPStr)] StringBuilder errorMessage, int maxMessageSize);

        //DLL int modbus_read_bits(modbus_t *ctx, int addr, int nb, uint8_t *dest);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_read_bits(ModbusHandle ctx, int addr, int nb, IntPtr dest);

        //DLL int modbus_read_input_bits(modbus_t *ctx, int addr, int nb, uint8_t *dest);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_read_input_bits(ModbusHandle ctx, int addr, int nb, IntPtr dest);

        //DLL int modbus_read_registers(modbus_t *ctx, int addr, int nb, uint16_t *dest);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_read_registers(ModbusHandle ctx, int addr, int nb, IntPtr dest);

        //DLL int modbus_read_input_registers(modbus_t *ctx, int addr, int nb, uint16_t *dest);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_read_input_registers(ModbusHandle ctx, int addr, int nb, IntPtr dest);

        //DLL int modbus_write_bit(modbus_t *ctx, int coil_addr, int status);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_write_bit(ModbusHandle ctx, int coil_addr, int status);

        //DLL int modbus_write_register(modbus_t *ctx, int reg_addr, int value);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_write_register(ModbusHandle ctx, int addr, int value);

        //DLL int modbus_write_bits(modbus_t *ctx, int addr, int nb, const uint8_t *data);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_write_bits(ModbusHandle ctx, int addr, int nb, IntPtr data);

        //DLL int modbus_write_registers(modbus_t *ctx, int addr, int nb, const uint16_t *data);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_write_registers(ModbusHandle ctx, int addr, int nb, IntPtr data);

        //DLL int modbus_write_and_read_registers(modbus_t *ctx, int write_addr, int write_nb,
        //                                    const uint16_t *src, int read_addr, int read_nb,
        //                                    uint16_t *dest);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_write_and_read_registers(ModbusHandle ctx,
            int write_addr, int write_nb, IntPtr src,
            int read_addr, int read_nb, IntPtr dest);

        //DLL int modbus_report_slave_id(modbus_t *ctx, uint8_t *dest);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_report_slave_id(ModbusHandle ctx, IntPtr dest);

        ////DLL modbus_mapping_t* modbus_mapping_new(int nb_coil_status, int nb_input_status,
        ////                                     int nb_holding_registers, int nb_input_registers);
        //[DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        //public static extern IntPtr modbus_mapping_new(int nb_coil_status, int nb_input_status,
        //                                      int nb_holding_registers, int nb_input_registers);
        ////DLL void modbus_mapping_free(modbus_mapping_t *mb_mapping);
        //[DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        //public static extern void modbus_mapping_free(ref ModbusMapping mb_mapping);

        //DLL int modbus_send_raw_request(ModbusHandle ctx, uint8_t *raw_req, int raw_req_length);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_send_raw_request(ModbusHandle ctx, IntPtr raw_req, int raw_req_length);

        ////DLL int modbus_receive(modbus_t *ctx, uint8_t *req);
        //[DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        //public static extern int modbus_receive(ModbusHandle ctx, IntPtr req);

        //// not implemented: int modbus_receive_from(modbus_t *ctx, int sockfd, uint8_t *req);

        //DLL int modbus_receive_confirmation(modbus_t *ctx, uint8_t *rsp);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int modbus_receive_confirmation(ModbusHandle ctx, IntPtr rsp);

        ////DLL int modbus_reply(modbus_t *ctx, const uint8_t *req,
        ////                     int req_length, modbus_mapping_t *mb_mapping);
        //[DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        //public static extern int modbus_reply(ModbusHandle ctx, IntPtr req,
        //                   int req_length, ref ModbusMapping mb_mapping);

        ////DLL int modbus_reply_exception(modbus_t *ctx, const uint8_t *req,
        ////                           unsigned int exception_code);
        //[DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        //public static extern int modbus_reply_exception(ModbusHandle ctx, IntPtr req,
        //                           uint exception_code);


        #endregion Modbus

        #region Modbus Utilities
        //DLL void modbus_set_bits_from_byte(uint8_t *dest, int index, const uint8_t value);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void modbus_set_bits_from_byte(IntPtr dest, int index, byte value);

        //DLL void modbus_set_bits_from_bytes(uint8_t *dest, int index, unsigned int nb_bits, const uint8_t *tab_byte);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void modbus_set_bits_from_bytes(IntPtr dest, int index, uint nb_bits, IntPtr tab_byte);

        //DLL uint8_t modbus_get_byte_from_bits(const uint8_t *src, int index, unsigned int nb_bits);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern byte modbus_get_byte_from_bits(IntPtr src, int index, uint nb_bits);

        //DLL float modbus_get_float(const uint16_t *src);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern float modbus_get_float(IntPtr src);

        //DLL void modbus_set_float(float f, uint16_t *dest);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void modbus_set_float(float f, IntPtr dest);

        #endregion Modbus Utilities


        #region TCP

        //DLL modbus_t* modbus_new_tcp(const char *ip_address, int port);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Ansi, BestFitMapping = false, ThrowOnUnmappableChar = true)]
        public static extern ModbusHandle modbus_new_tcp(
            [MarshalAs(UnmanagedType.LPStr)] string ip_address,
            int port);

        //DLL int modbus_tcp_listen(modbus_t *ctx, int nb_connection);
        //DLL int modbus_tcp_accept(modbus_t *ctx, int *socket);

        //DLL modbus_t* modbus_new_tcp_pi(const char *node, const char *service);
        //DLL int modbus_tcp_pi_listen(modbus_t *ctx, int nb_connection);
        //DLL int modbus_tcp_pi_accept(modbus_t *ctx, int *socket);

        #endregion TCP



        #region RTU

        //DLL modbus_t* modbus_new_rtu(const char *device, int baud, char parity,
        //                         int data_bit, int stop_bit);
        [DllImport("libmodbus.dll", CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Ansi, BestFitMapping = false, ThrowOnUnmappableChar = true)]
        public static extern ModbusHandle modbus_new_rtu([MarshalAs(UnmanagedType.LPStr)] string device,
                                                         int baud, char parity, int data_bit,
                                                         int stop_bit);

        #endregion RTU


    }
}
