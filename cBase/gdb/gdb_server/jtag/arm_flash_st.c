/*
 * arm_flash_st.c
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
#include <sys/time.h>

#include <sysexits.h>
#include <ctype.h>
#include <unistd.h>

#include "dbg_msg.h"
#include "jt_arm.h"
#include "jt_flash.h"
#include "arm_gdbstub.h"


/*
 * SGS Thomson flash program
 *
 * return length of prorammed data (in bytes)
 */
int stFlashProgram(uint32_t addr, int maxSize, uint32_t *data)
{
	struct memMap *ws;
	int block_len, len;
	int block_cnt;
	volatile uint32_t val;
	struct timeval time_begin, time_curr;
	uint32_t save_CPSR;
		
	ws = getWorkSpace();
	/*we need the work space */
	if( ws != NULL )
	{
		addr &= 0x001fFFFCul;

		block_len = PAGE_SIZE/sizeof(uint32_t);	// = 1024 words
		if(maxSize < block_len*4)
			block_len = maxSize/sizeof(uint32_t);

		/*convert 32bit-sized -block_len- into 64bit-sized -block_cnt- */
		block_cnt = block_len/2;
		len = block_len;
		
		/*check if we are able to load the dcc target prog*/
		if ( useWorkspace(WORKSPACE_ALGO_FLASH_ST, addr, 8*1024) == 0 )
		{
			save_CPSR = CPU.CPSR;
			CPU.CPSR = 0xd3; // IRQ and FRQ disabled; Supervisor mode; ARM - State
			IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
				fputc('#',stdout);
			/*DCC control*/
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
			if(val & 0x2) // read out data from target
				jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_DATA);
			jtag_arm_ClearAnyBreakPoint();
			jtag_arm_RunProgram(ws->baseAddr + ws->memBufferType.Workspace.offset);
			jtag_arm_enterMonitorMode();

			/*prepare to write data to target*/
			gettimeofday( &time_begin, NULL);
			jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
			do{
				val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
				
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 5msec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				    + (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 5 ) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"timeout did not get ready to write\n");
					jtag_arm_Mointor2DebugMode();
					while(jtag_arm_PollDbgState() == 0)
						;
					/*restore regs*/
					CPU.CPSR = save_CPSR;
					return 0;
				}
			}while(val & 1);
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 0x4D524140);	// "@ARM" = 40 41 52 4D
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, addr);	// addr bit[0..1] == 00 -> cmd program
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, block_cnt);
			/*read "ack!" = 61 63 6b 21 */
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_DATA); 
			if(val != 0x216b6361)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing ACK\n");
				jtag_arm_Mointor2DebugMode();
				while(jtag_arm_PollDbgState() == 0)
					;
				/*restore regs*/
				CPU.CPSR = save_CPSR;
				return 0;
			}
			/*write data*/
			do{
				val = *data++;
				jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, val);
				
				jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
				/*delay by reading DCC status*/
				gettimeofday( &time_begin, NULL);
				do
				{
					val =jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
					
					gettimeofday(&time_curr, NULL);
					/* timeout if more than 25msec*/
					if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
					    + (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 25 ) 
					{
						dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"write timeout\n");
						if(val & 0x2)
						{
							/*error message*/
							val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_DATA);
							dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"write error: 0x%X\n",val);
						}
						jtag_arm_Mointor2DebugMode();
						while(jtag_arm_PollDbgState() == 0)
							;
						/*restore regs*/
						CPU.CPSR = save_CPSR;
						return 0;
					}
				} while(val & 0x1);
			}while(--len);
		
			jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
			/*delay by reading DCC status*/
			gettimeofday( &time_begin, NULL);
			for(;;)
			{
				val =jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
					
				if(val & 0x2)
					break;
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 5msec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				    + (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 5 ) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"timeout no reponse\n");
					jtag_arm_Mointor2DebugMode();
					while(jtag_arm_PollDbgState() == 0)
							;
					/*restore regs*/
					CPU.CPSR = save_CPSR;
					return 0;
				}
			}

			/*read "fin!" = 66 69 6e 21 */
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_DATA); 
			
			/*switch back to debug mode*/
			//jtag_arm_PutAnyBreakPoint();
			jtag_arm_Mointor2DebugMode();
			while(jtag_arm_PollDbgState() == 0)
				;
			/*restore regs*/
			CPU.CPSR = save_CPSR;
			if(val != 0x216e6966)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing fin! write error: 0x%X\n",val);
				return 0; /*fail*/
			}
			/*next addr*/
			return block_len * sizeof(uint32_t);
		}
	}
	/*no way to use the jtag interface directly*/
	IF_DBG(DBG_LEVEL_GDB_ARM_ERROR)
		fputc('?',stdout);
	return 0; /*fail*/
}


/*
 * SGS Thomson flash erase
 *
 * return 0 on success 
 * 	  else errro number
 */
int stFlashErase(uint32_t addr, int idx)
{
	struct memMap *ws;
	uint32_t erase_msk;
	volatile uint32_t val;
	struct timeval time_begin, time_curr;
	uint32_t save_CPSR;
		
	
	ws = getWorkSpace();
	if( ws != NULL )
	{
		addr &= 0x001fFFFCul;
		
		/*set which sector to erase*/
		if(addr & 0x100000)
		{
			/*Bank 1 (16k Data memory)*/
			if (idx > 1)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"wrong Bank 1 sector %d\n",idx);
				return 7;
			}
			erase_msk = 1 << (idx + 16);
		}
		else
		{
			/*Bank 0 (256K Progam memory)*/
			if (idx > 7)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"wrong Bank 0 sector %d\n",idx);
				return 6;
			}
			erase_msk = 1 << idx;
		}
		
		/*check if we are able to load the dcc target prog*/
		if ( useWorkspace(WORKSPACE_ALGO_FLASH_ST, addr, 8*1024) == 0 )
		{
			save_CPSR = CPU.CPSR;
			CPU.CPSR = 0xd3; // IRQ and FRQ disabled; Supervisor mode; ARM - State
			IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
				fputc('#',stdout);
			/*DCC control*/
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
			if(val & 0x2) // read out data from target
				jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_DATA);
			jtag_arm_ClearAnyBreakPoint();
			jtag_arm_RunProgram(ws->baseAddr + ws->memBufferType.Workspace.offset);
			jtag_arm_enterMonitorMode();

			/*write data to target*/
			gettimeofday( &time_begin, NULL);
			jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
			do{
				val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
				
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 5msec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				    + (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 5 ) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"timeout did not get ready to erase\n");
					jtag_arm_Mointor2DebugMode();
					while(jtag_arm_PollDbgState() == 0)
						;
					/*restore regs*/
					CPU.CPSR = save_CPSR;
					return 5;
				}
			}while(val & 1);
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 0x4D524140);	// "@ARM" = 40 41 52 4D
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, addr | 1);	// addr bit[0..1] == 01 -> cmd erase
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, erase_msk);
			/*read "ack!" = 61 63 6b 21 */
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_DATA); 
			if(val != 0x216b6361)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing ACK\n");
				jtag_arm_Mointor2DebugMode();
				while(jtag_arm_PollDbgState() == 0)
					;
				/*restore regs*/
				CPU.CPSR = save_CPSR;
				return 4;
			}
		
			jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
			/*delay by reading DCC status*/
			gettimeofday( &time_begin, NULL);
			for(;;)
			{
				val =jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
					
				if(val & 0x2)
					break;
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 500msec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				    + (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 500 ) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"timeout no reponse\n");
					jtag_arm_Mointor2DebugMode();
					while(jtag_arm_PollDbgState() == 0)
							;
					/*restore regs*/
					CPU.CPSR = save_CPSR;
					return 3;
				}
			}

			/*read "fin!" = 66 69 6e 21 */
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_DATA); 
			
			/*switch back to debug mode*/
			//jtag_arm_PutAnyBreakPoint();
			jtag_arm_Mointor2DebugMode();
			while(jtag_arm_PollDbgState() == 0)
				;
			/*restore regs*/
			CPU.CPSR = save_CPSR;
			if(val != 0x216e6966)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing fin! erase error: 0x%X\n",val);
				return 2; /*fail*/
			}
			/*next addr*/
			return 0;
		}
	}
	/*no way to use the jtag interface directly*/
	IF_DBG(DBG_LEVEL_GDB_ARM_ERROR)
		fputc('?',stdout);
	return 1; /*fail*/
}



