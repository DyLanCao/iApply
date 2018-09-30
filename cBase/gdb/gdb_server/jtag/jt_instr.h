/*
 * jt_instr.h
 *
 * Intructions send to the TAP Controller
 * 
 * Support functions to access the JTAG interface via parallel port
 *
 * Copyright (C) 2005,2006
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Written by Thomas Klein <ThRKlein@users.sf.net>, 2005.
 * 
 */


extern const char instr_extest[];
extern const char instr_scan_n[]; 
extern const char instr_sample[]; 
extern const char instr_restart[];
extern const char instr_clamp[];
extern const char instr_highz[];
extern const char instr_clampz[];
extern const char instr_intest[];
extern const char instr_idcode[];
extern const char instr_bypass[];

extern const int instr_length;

/*
 * scan chain 0 (113 bits) - Macrocell Scan Test
 * scan chain 1 ( 33 bits) - Debug
 * scan chain 2 ( 38 bits) - Embedded ICE logic
 */

#define SCAN_CHAIN_NR_MACROCELL		0
#define SCAN_CHAIN_NR_DEBUG		1
#define SCAN_CHAIN_NR_EMBEDDED_ICE	2

extern int scan_chain;
extern const int data_length_of_scan_chain[];

extern char jatag_data_IDcode[];
extern const int data_length_of_IDCODE; /*(4 bit)-Version (16 bit)-Part number (11 bit)-VerdorID (1 bit)-End marker*/

extern int active_jtag_device;

extern const int    idcode_list_len ;
extern char * idcode_string[2]; 

extern int jtag_identify_devices(void);
extern int jtag_send_instr(const char * cmd);
extern int jtag_exchange_data(int len, char *data_out, char *data_in);
extern void jtag_eos(void);
extern void jtag_start(void);
extern void jtag_reset(void);
extern void jtag_hard_reset(void);
extern void jtag_linger(int delay);

