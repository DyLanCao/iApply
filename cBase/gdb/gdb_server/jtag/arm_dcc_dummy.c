/*
 * arm_dcc_dummy.c
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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <sysexits.h>
#include <ctype.h>
#include <unistd.h>

#include "dbg_msg.h"
#include "jt_arm.h"
#include "jt_flash.h"
#include "arm_gdbstub.h"


/*
 *
 */
void gdb_dcc_dummy(uint32_t len)
{	
	struct memMap *ws;
	volatile uint32_t val;
	uint32_t save_CPSR;

	/*make sure len isn't zero*/
	if(len == 0)
		len = 1;
	
	ws = getWorkSpace();
	/*check if we are able to load the dcc target prog*/
	if(ws != NULL && useWorkspace(WORKSPACE_ALGO_DUMMY, 0, 1024) == 0 )
	{
		save_CPSR = CPU.CPSR;
		CPU.CPSR = 0xd3; // IRQ and FRQ disabled; Supervisor mode; ARM - State
		IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
			fputc('~',stdout);
		/*DCC control*/
		val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
		if(val & 0x2) // read out data from target
			jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_DATA);
		jtag_arm_ClearAnyBreakPoint();
		jtag_arm_RunProgram(ws->baseAddr + ws->memBufferType.Workspace.offset);
		//jtag_arm_enterMonitorMode();
		
		/*write data to target*/
		jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
		do{
			val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
		}while(val & 1);
		jtag_arm_IceRT_RegWrite(5, 0x4D524140);// "@ARM" = 40 41 52 4D
		jtag_arm_IceRT_RegWrite(5, len);

		jtag_arm_IceRT_RegRead_Once(5); //send read command
		/*receive response*/
		do{
			if(len == 1)
				val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
			else
				val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_DATA);
			if(len != val)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN
					,"len (%d) != (%d)\n"
					,len,val);
				break;
			}
			len--;
		}while(len);
		/*switch back to debug mode*/
		//jtag_arm_PutAnyBreakPoint();
		jtag_arm_Mointor2DebugMode();
		while(jtag_arm_PollDbgState() == 0)
			;
		/*restore regs*/
		CPU.CPSR = save_CPSR;
		return;
	}
	IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
		fputc('?',stdout);
	return;
}



