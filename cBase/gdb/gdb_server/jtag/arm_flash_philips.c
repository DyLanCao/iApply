/*
 * arm_flash_philips.c
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


/********************************/
uint32_t	LPC_frequence = 0;
int 		LPC_boot_sector_enabled = 0;

/*
 * Philips flash
 * program 512 bytes per 128 bit size
 *
 * return length of prorammed data (in bytes)
 */
int philipsFlashProgram(uint32_t addr, int maxSize, uint32_t *data)
{
	struct memMap *ws;
	int block_len;
	int block_cnt;
	int x;
	volatile uint32_t val;
	struct timeval time_begin, time_curr;
	uint32_t save_CPSR;
		
	/*check frequence value. Must be in range of 1MHz - 75MHz*/
	if(LPC_frequence < 1000 || LPC_frequence > 75000)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"no valid LPC_frequence %d kHz\n",LPC_frequence);
		return 0;
	}
	
	ws = getWorkSpace();
	/*we need the work space and the address start must be at a 512 byte boundary*/
	if( (ws != NULL && (addr & 0x1FF) == 0 ))
	{
		block_len = 128;	// 128 words = 512 Bytes

		if(maxSize < block_len * 4)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"only sizes of 512Bytes per page are supported\n");
			return 0;
		}

		/*convert 32bit-sized -block_len- into 128bit-sized -block_cnt- */
		block_cnt = block_len/4; 
		
		/*check if we are able to load the dcc target prog*/
		if ( useWorkspace(WORKSPACE_ALGO_FLASH_PHILIPS, addr, 8*1024) == 0 )
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
			jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
			do{
				val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
			}while(val & 1);
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 0x4D524140);// "@ARM" = 40 41 52 4D
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, addr);
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, LPC_frequence / 200);
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, (LPC_frequence / 2048 ) + 1 );
			x = LPC_frequence * 0.22; 
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, x);
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
				jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
				val = *data++;
				jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, val);
				val = *data++;
				jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, val);
				val = *data++;
				jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, val);
				val = *data++;
				jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, val);
			}while(--block_cnt);
			/*delay by reading DCC status*/
			gettimeofday( &time_begin, NULL);
			for(;;)
			{
				val =jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
				if(val & 0x2)
					break;
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 5msec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				+ (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 5 ) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"write timeout ");
					break;
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
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing fin!\n");
				return 0; /*fail*/
			}
			/*next addr*/
			return block_len * 4;
		}
	}
	/*no way to use the jtag interface directly*/
	IF_DBG(DBG_LEVEL_GDB_ARM_ERROR)
		fputc('?',stdout);
	return 0; /*fail*/
}

/*
 * Philips flash unlock
 *
 * return 0 on success 
 * 	  else errro number
 */
int philipsFlashUnlock(uint32_t flash_base_addr, uint32_t mask)
{
	struct memMap *ws;
	volatile uint32_t val;
	struct timeval time_begin, time_curr;
	uint32_t save_CPSR;
		
	ws = getWorkSpace();
	/*we need the work space*/
	if( ws != NULL )
	{
		/*check if we are able to load the dcc target prog*/
		if ( useWorkspace(WORKSPACE_ALGO_FLASH_PHILIPS, flash_base_addr, 8*1024) == 0 )
		{
			save_CPSR = CPU.CPSR;
			CPU.CPSR = 0xd3; // IRQ and FRQ disabled; Supervisor mode; ARM - State
			IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
				fputc('.',stdout);
			/*DCC control*/
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
			if(val & 0x2) // read out data from target
				jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_DATA);
			jtag_arm_ClearAnyBreakPoint();
			jtag_arm_RunProgram(ws->baseAddr + ws->memBufferType.Workspace.offset);
			jtag_arm_enterMonitorMode();
			
			/*write data to target*/
			jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
			do{
				val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
			}while(val & 1);
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 0x4D524140);// "@ARM" = 40 41 52 4D
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 2);		// unlock command
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, mask);
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 0);
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 50);
			
			/*read "ack!" = 61 63 6b 21 */
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_DATA); 
			if(val != 0x216b6361) // "ack!" = 61 63 6b 21
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing ACK\n");
				jtag_arm_Mointor2DebugMode();
				while(jtag_arm_PollDbgState() == 0)
					;
				/*restore regs*/
				CPU.CPSR = save_CPSR;
				return 2;
			}
			/*delay by reading DCC status*/
			gettimeofday( &time_begin, NULL);
			for(;;)
			{
				val =jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
				if(val & 0x2)
					break;
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 2msec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				+ (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 2 ) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"unlock timeout ");
					break;
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
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing fin!\n");
				return 3; /*fail*/
			}

			return 0;
		}
	}
	/*no way to use the jtag interface directly*/
	IF_DBG(DBG_LEVEL_GDB_ARM_ERROR)
		fputc('?',stdout);
	return 1; /*fail*/
}

/*
 * Philips flash lock
 *
 * return 0 on success 
 * 	  else errro number
 */
int philipsFlashLock(uint32_t flash_base_addr, uint32_t mask)
{
	struct memMap *ws;
	volatile uint32_t val;
	struct timeval time_begin, time_curr;
	uint32_t save_CPSR;
		
	ws = getWorkSpace();
	/*we need the work space*/
	if( ws != NULL )
	{
		/*check if we are able to load the dcc target prog*/
		if ( useWorkspace(WORKSPACE_ALGO_FLASH_PHILIPS, flash_base_addr, 8*1024) == 0 )
		{
			save_CPSR = CPU.CPSR;
			CPU.CPSR = 0xd3; // IRQ and FRQ disabled; Supervisor mode; ARM - State
			IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
				fputc('.',stdout);
			/*DCC control*/
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
			if(val & 0x2) // read out data from target
				jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_DATA);
			jtag_arm_ClearAnyBreakPoint();
			jtag_arm_RunProgram(ws->baseAddr + ws->memBufferType.Workspace.offset);
			jtag_arm_enterMonitorMode();
			
			/*write data to target*/
			jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
			do{
				val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
			}while(val & 1);
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 0x4D524140);// "@ARM" = 40 41 52 4D
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 3);		// lock command
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, mask);
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 0);
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 50);
			/*read "ack!" = 61 63 6b 21 */
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_DATA); 
			if(val != 0x216b6361) // "ack!" = 61 63 6b 21
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing ACK\n");
				jtag_arm_Mointor2DebugMode();
				while(jtag_arm_PollDbgState() == 0)
					;
				/*restore regs*/
				CPU.CPSR = save_CPSR;
				return 2;
			}

			/*delay by reading DCC status*/
			gettimeofday( &time_begin, NULL);
			for(;;)
			{
				val =jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
				if(val & 0x2)
					break;
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 2msec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				+ (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 1 ) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"lock timeout ");
					break;
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
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing fin!\n");
				return 3; /*fail*/
			}

			return 0;
		}
	}
	/*no way to use the jtag interface directly*/
	IF_DBG(DBG_LEVEL_GDB_ARM_ERROR)
		fputc('?',stdout);
	return 1; /*fail*/
}


/*
 * Philips flash erase
 *
 * return 0 on success 
 * 	  else errro number
 */
int philipsFlashEraseAllUnlocked(uint32_t flash_base_addr)
{
	struct memMap *ws;
	int x;
	volatile uint32_t val;
	struct timeval time_begin, time_curr;
	uint32_t save_CPSR;
		
	/*check frequence value. Must be in range of 1MHz - 75MHz*/
	if(LPC_frequence < 1000 || LPC_frequence > 75000)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"no valid LPC_frequence %d kHz\n",LPC_frequence);
		return 1;
	}
	
	ws = getWorkSpace();
	/*we need the work space*/
	if( ws != NULL )
	{
		/*check if we are able to load the dcc target prog*/
		if ( useWorkspace(WORKSPACE_ALGO_FLASH_PHILIPS, flash_base_addr, 8*1024) == 0 )
		{
			save_CPSR = CPU.CPSR;
			CPU.CPSR = 0xd3; // IRQ and FRQ disabled; Supervisor mode; ARM - State
			IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
				fputc('.',stdout);
			/*DCC control*/
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
			if(val & 0x2) // read out data from target
				jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_DATA);
			jtag_arm_ClearAnyBreakPoint();
			jtag_arm_RunProgram(ws->baseAddr + ws->memBufferType.Workspace.offset);
			jtag_arm_enterMonitorMode();
			
			/*write data to target*/
			jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
			do{
				val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
			}while(val & 1);
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 0x4D524140);// "@ARM" = 40 41 52 4D
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 1);		// erase command
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, LPC_frequence / 200);
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, (LPC_frequence * 25 / 128 ) + 1 );
			x = LPC_frequence * 80.02; 
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, x);
			/*read "ack!" = 61 63 6b 21 */
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_DATA); 
			if(val != 0x216b6361) // "ack!" = 61 63 6b 21
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing ACK\n");
				jtag_arm_Mointor2DebugMode();
				while(jtag_arm_PollDbgState() == 0)
					;
				/*restore regs*/
				CPU.CPSR = save_CPSR;
				return 2;
			}
			/*delay by reading DCC status*/
			gettimeofday( &time_begin, NULL);
			for(;;)
			{
				val =jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
				if(val & 0x2)
					break;
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 400msec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				+ (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 400 ) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"erase timeout ");
					break;
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
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing fin!\n");
				return 3; /*fail*/
			}

			return 0;
		}
	}
	/*no way to use the jtag interface directly*/
	IF_DBG(DBG_LEVEL_GDB_ARM_ERROR)
		fputc('?',stdout);
	return 1; /*fail*/
}



