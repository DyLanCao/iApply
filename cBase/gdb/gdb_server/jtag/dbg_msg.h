/*
 * dbg_msg.h
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
#include <stdint.h>
#include <stdarg.h>
#include <sysexits.h>

enum VerboseLevel {
	LESS_VERBOSE = 0,
	MORE_VERBOSE = 1,
	INFO_VERBOSE = 2
};
extern enum VerboseLevel verbose;

#define error(_X_) do { fprintf(stderr,(_X_)); exit(EX_OSERR);}while(0)
#define error2(_X_,_Y_) do { fprintf(stderr,(_X_),(_Y_)); exit(EX_OSERR);}while(0)

extern uint32_t dbg_msg_msk;

#define DBG_LEVEL_JTAG_TAP		0x0001
#define DBG_LEVEL_JTAG_INSTR		0x0002
#define DBG_LEVEL_JTAG_ICERT_LOW	0x0004
#define DBG_LEVEL_JTAG_ICERT		0x0008
#define DBG_LEVEL_JTAG_ARM_LOW		0x0010
#define DBG_LEVEL_JTAG_ARM		0x0020
#define DBG_LEVEL_GDB_ARM_INFO_LOW	0x0040
#define DBG_LEVEL_GDB_ARM_INFO		0x0080
#define DBG_LEVEL_GDB_ARM_WARN		0x0100
#define DBG_LEVEL_GDB_ARM_ERROR		0x0400

#define DBG_LEVEL_ALL	( DBG_LEVEL_GDB_ARM_ERROR    | \
			  DBG_LEVEL_GDB_ARM_WARN     | \
			  DBG_LEVEL_GDB_ARM_INFO     | \
			  DBG_LEVEL_GDB_ARM_INFO_LOW | \
			  DBG_LEVEL_JTAG_ARM         | \
			  DBG_LEVEL_JTAG_ARM_LOW     | \
			  DBG_LEVEL_JTAG_ICERT       | \
			  DBG_LEVEL_JTAG_ICERT_LOW   | \
			  DBG_LEVEL_JTAG_INSTR       | \
			  DBG_LEVEL_JTAG_TAP           \
			)

#define DBG( _level_ , _fct_ )	if ( ((uint32_t)(_level_) & dbg_msg_msk) != 0 ) do { _fct_;} while (0)
#define IF_DBG( _level_ )	if ( ((uint32_t)(_level_) & dbg_msg_msk) != 0 )

extern void dbgPrintf(uint32_t dbg_level, char *  fmt, ...);

