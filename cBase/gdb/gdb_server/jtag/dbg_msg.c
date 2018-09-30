/*
 * dbg_msg.c
 *
 * Copyright (C) 2005
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
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include <stdarg.h>
#include <sysexits.h>
#include "dbg_msg.h"

enum VerboseLevel verbose = LESS_VERBOSE;
uint32_t dbg_msg_msk = 0;

void dbgPrintf(uint32_t dbg_level, char *  fmt, ...)
{
	if ( dbg_level & dbg_msg_msk)
	{
		va_list argp;

		va_start(argp, fmt);
		vfprintf(stdout, fmt, argp);
		va_end(argp);
	}
	return;
}

