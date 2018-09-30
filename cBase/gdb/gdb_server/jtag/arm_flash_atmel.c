/*
 * arm_flash_atmel.c
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


#define SAM7_MC_FMR__FMCN(_x_)	(((_x_)&0xFF)<<16)
#define SAM7_MC_FMR__FWS(_x_)	(((_x_)&0x3)<<8)
#define SAM7_MC_FMR__NEBP(_x_)	(((_x_)&0x1)<<7)

#define SAM7_MC_FCR__KEY	(0x5A<<24)
#define SAM7_MC_FCR__PAGEN(_x_)	(((_x_)&0x3FF)<<8)
#define SAM7_MC_FCR__FCMD(_x_)	(((_x_)&0xF))
#define SAM7_MC_FCR__FCMD_WP	1
#define SAM7_MC_FCR__FCMD_SLB	2
#define SAM7_MC_FCR__FCMD_WPL	3
#define SAM7_MC_FCR__FCMD_CLB	4
#define SAM7_MC_FCR__FCMD_EA	8
#define SAM7_MC_FCR__FCMD_SGPB	11
#define SAM7_MC_FCR__FCMD_CGPB	13
#define SAM7_MC_FCR__FCMD_SSB	15

/********************************/
uint32_t	SAM7_frequence = 0;

/*
 * Atmel SAM7 flash
 * program one page
 *
 * return length of prorammed data (in bytes)
 */
int atmelFlashEraseAndProgram(uint32_t addr, int pageNumber, int pageSize, uint32_t *data)
{
	struct memMap *ws;
	int block_len;
	int block_cnt;
	uint32_t num_clkTicksPerMicroSec;	// FMCN
	uint32_t clk_wait_mode;			// FWS
	uint32_t mc_fmr, mc_fcr;
	volatile uint32_t val;
	struct timeval time_begin, time_curr;
	uint32_t save_CPSR;
		
	/*check frequence value. Must be in range of 1kHz - 55MHz*/
	if(SAM7_frequence < 1 || SAM7_frequence > 55000)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"no valid SAM7_frequence %d kHz\n",LPC_frequence);
		return 0;
	}

	if(SAM7_frequence > 30000)
	{
		clk_wait_mode = 1; // 3 cycles
		num_clkTicksPerMicroSec = (SAM7_frequence  / 666) + 1;
	}
	else
	{
		clk_wait_mode = 0; // 2 cycles
		num_clkTicksPerMicroSec = (SAM7_frequence / 666) + 1;
	}
	/*setup flash mode register */
	mc_fmr	= SAM7_MC_FMR__FMCN(num_clkTicksPerMicroSec)	// Flash microsecond cycle number
		| SAM7_MC_FMR__FWS(clk_wait_mode)		// Flash wait state
		| SAM7_MC_FMR__NEBP(0);				// Erase page before progamming
	/*setup flash command register*/
	mc_fcr	= SAM7_MC_FCR__KEY
		| SAM7_MC_FCR__PAGEN(pageNumber)
		| SAM7_MC_FCR__FCMD(SAM7_MC_FCR__FCMD_WP);
	
	ws = getWorkSpace();
	/*we need the work space and the address start must be at a 128 byte boundary*/
	if( (ws != NULL && (addr & 0x7F) == 0 ))
	{
		block_len = pageSize/4; 
		block_cnt = block_len; 
		
		/*check if we are able to load the dcc target prog*/
		if ( useWorkspace(WORKSPACE_ALGO_FLASH_ATMEL, addr, 8*1024) == 0 )
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
			gettimeofday(&time_curr, NULL);
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
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 0x4D524140);// "@ARM" = 40 41 52 4D
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, mc_fmr); // mode
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, mc_fcr); // cmd
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, addr); // addr
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, block_len); // len
			
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
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"ack timeout\n");
					break;
				}
			}

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
			}while(--block_cnt);
			/*delay by reading DCC status*/
			gettimeofday( &time_begin, NULL);
			for(;;)
			{
				val =jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
				if(val & 0x2)
					break;
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 1500msec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				+ (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 1500 ) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"write timeout\n");
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
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing fin!(0x%X)\n",val);
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
 * Atmel SAM7 flash
 * program one page
 *
 * return length of prorammed data (in bytes)
 */
int atmelFlashProgramOnly(uint32_t addr, int pageNumber, int pageSize, uint32_t *data)
{
	struct memMap *ws;
	int block_len;
	int block_cnt;
	uint32_t num_clkTicksPerMicroSec;	// FMCN
	uint32_t clk_wait_mode;			// FWS
	uint32_t mc_fmr, mc_fcr;
	volatile uint32_t val;
	struct timeval time_begin, time_curr;
	uint32_t save_CPSR;
		
	/*check frequence value. Must be in range of 1kHz - 55MHz*/
	if(SAM7_frequence < 1 || SAM7_frequence > 55000)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"no valid SAM7_frequence %d kHz\n",LPC_frequence);
		return 0;
	}

	if(SAM7_frequence > 30000)
	{
		clk_wait_mode = 1; // 3 cycles
		num_clkTicksPerMicroSec = ((SAM7_frequence) / 666) + 1;
	}
	else
	{
		clk_wait_mode = 0; // 2 cycles
		num_clkTicksPerMicroSec = ((SAM7_frequence) / 666) + 1;
	}
	/*setup flash mode register */
	mc_fmr	= SAM7_MC_FMR__FMCN(num_clkTicksPerMicroSec)	// Flash microsecond cycle number
		| SAM7_MC_FMR__FWS(clk_wait_mode)		// Flash wait state
		| SAM7_MC_FMR__NEBP(1);				// No Erase before progamming
	/*setup flash command register*/
	mc_fcr	= SAM7_MC_FCR__KEY
		| SAM7_MC_FCR__PAGEN(pageNumber)
		| SAM7_MC_FCR__FCMD(SAM7_MC_FCR__FCMD_WP);
	
	ws = getWorkSpace();
	/*we need the work space and the address start must be at a 128 byte boundary*/
	if( (ws != NULL && (addr & 0x7F) == 0 ))
	{
		block_len = pageSize/4; 
		block_cnt = block_len; 
		
		/*check if we are able to load the dcc target prog*/
		if ( useWorkspace(WORKSPACE_ALGO_FLASH_ATMEL, addr, 8*1024) == 0 )
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
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"timeout did not get ready to write\n");
					jtag_arm_Mointor2DebugMode();
					while(jtag_arm_PollDbgState() == 0)
						;
					/*restore regs*/
					CPU.CPSR = save_CPSR;
					return 0;
				}
			}while(val & 1);
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 0x4D524140);// "@ARM" = 40 41 52 4D
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, mc_fmr); // mode
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, mc_fcr); // cmd
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, addr); // addr
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, block_len); // len
			
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
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"ack timeout\n");
					break;
				}
			}

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
			}while(--block_cnt);
			/*delay by reading DCC status*/
			gettimeofday( &time_begin, NULL);
			for(;;)
			{
				val =jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
				if(val & 0x2)
					break;
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 500msec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				+ (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 500 ) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"write timeout\n");
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
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing fin!(0x%X)\n",val);
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
 * Atmel flash unlock
 *
 * return 0 on success 
 * 	  else errro number
 */
int atmelFlashUnlock(uint32_t addr, int pageNumber)
{
	struct memMap *ws;
	uint32_t num_clkTicksPerMicroSec;	// FMCN
	uint32_t clk_wait_mode;			// FWS
	uint32_t mc_fmr, mc_fcr;
	volatile uint32_t val;
	struct timeval time_begin, time_curr;
	uint32_t save_CPSR;
		
	/*check frequence value. Must be in range of 1kHz - 55MHz*/
	if(SAM7_frequence < 1 || SAM7_frequence > 55000)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"no valid SAM7_frequence %d kHz\n",LPC_frequence);
		return 2;
	}

	if(SAM7_frequence > 30000)
	{
		clk_wait_mode = 1; // 3 cycles
		num_clkTicksPerMicroSec = ((SAM7_frequence) / 1000) + 1;
	}
	else
	{
		clk_wait_mode = 0; // 2 cycles
		num_clkTicksPerMicroSec = ((SAM7_frequence) / 1000) + 1;
	}
	
	/*setup flash mode register */
	mc_fmr	= SAM7_MC_FMR__FMCN(num_clkTicksPerMicroSec)	// Flash microsecond cycle number
		| SAM7_MC_FMR__FWS(clk_wait_mode)		// Flash wait state
		| SAM7_MC_FMR__NEBP(1);				// No Erase before progamming
	/*setup flash command register*/
	mc_fcr	= SAM7_MC_FCR__KEY
		| SAM7_MC_FCR__PAGEN(pageNumber)
		| SAM7_MC_FCR__FCMD(SAM7_MC_FCR__FCMD_CLB);
		
	ws = getWorkSpace();
	/*we need the work space*/
	if( ws != NULL )
	{
		/*check if we are able to load the dcc target prog*/
		if ( useWorkspace(WORKSPACE_ALGO_FLASH_ATMEL, addr, 8*1024) == 0 )
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
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, mc_fmr); // mode
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, mc_fcr); // cmd
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, addr); // addr
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 0); // len
			
			/*delay by reading DCC status*/
			gettimeofday( &time_begin, NULL);
			for(;;)
			{
				val =jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
				if(val & 0x2)
					break;
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 500msec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				+ (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 500 ) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"ack timeout\n");
					break;
				}
			}

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
				return 3;
			}
			/*delay by reading DCC status*/
			gettimeofday( &time_begin, NULL);
			for(;;)
			{
				val =jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
				if(val & 0x2)
					break;
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 500msec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				+ (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 500 ) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"write timeout\n");
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
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing fin!(0x%X)\n",val);
				return 4; /*fail*/
			}
			/*OK*/
			return 0;
		}
	}
	/*no way to use the jtag interface directly*/
	IF_DBG(DBG_LEVEL_GDB_ARM_ERROR)
		fputc('?',stdout);
	return 1; /*fail*/
}

/*
 * Atmel flash lock
 *
 * return 0 on success 
 * 	  else errro number
 */
int atmelFlashLock(uint32_t addr, int pageNumber)
{
	struct memMap *ws;
	uint32_t num_clkTicksPerMicroSec;	// FMCN
	uint32_t clk_wait_mode;			// FWS
	uint32_t mc_fmr, mc_fcr;
	volatile uint32_t val;
	struct timeval time_begin, time_curr;
	uint32_t save_CPSR;
		
	/*check frequence value. Must be in range of 1kHz - 55MHz*/
	if(SAM7_frequence < 1 || SAM7_frequence > 55000)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"no valid SAM7_frequence %d kHz\n",LPC_frequence);
		return 2;
	}

	if(SAM7_frequence > 30000)
	{
		clk_wait_mode = 1; // 3 cycles
		num_clkTicksPerMicroSec = ((SAM7_frequence) / 1000) + 1;
	}
	else
	{
		clk_wait_mode = 0; // 2 cycles
		num_clkTicksPerMicroSec = ((SAM7_frequence) / 1000) + 1;
	}
	
	/*setup flash mode register */
	mc_fmr	= SAM7_MC_FMR__FMCN(num_clkTicksPerMicroSec)	// Flash microsecond cycle number
		| SAM7_MC_FMR__FWS(clk_wait_mode)		// Flash wait state
		| SAM7_MC_FMR__NEBP(1);				// No Erase before progamming
	/*setup flash command register*/
	mc_fcr	= SAM7_MC_FCR__KEY
		| SAM7_MC_FCR__PAGEN(pageNumber)
		| SAM7_MC_FCR__FCMD(SAM7_MC_FCR__FCMD_SLB);
		
	ws = getWorkSpace();
	/*we need the work space*/
	if( ws != NULL )
	{
		/*check if we are able to load the dcc target prog*/
		if ( useWorkspace(WORKSPACE_ALGO_FLASH_ATMEL, addr, 8*1024) == 0 )
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
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, mc_fmr); // mode
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, mc_fcr); // cmd
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, addr); // addr
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 0); // len
			
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
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"ack timeout\n");
					break;
				}
			}

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
				return 3;
			}
			/*delay by reading DCC status*/
			gettimeofday( &time_begin, NULL);
			for(;;)
			{
				val =jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
				if(val & 0x2)
					break;
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 500msec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				+ (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 500 ) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"write timeout\n");
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
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing fin!(0x%X)\n",val);
				return 4; /*fail*/
			}
			/*OK*/
			return 0;
		}
	}
	/*no way to use the jtag interface directly*/
	IF_DBG(DBG_LEVEL_GDB_ARM_ERROR)
		fputc('?',stdout);
	return 1; /*fail*/
}

/*
 * Atmel SAM7 flash
 * erase one page
 * 
 * return 0 on success 
 * 	  else errro number
 */
int atmelFlashErase(uint32_t addr, int pageNumber)
{
	struct memMap *ws;
	uint32_t num_clkTicksPerMicroSec;	// FMCN
	uint32_t clk_wait_mode;			// FWS
	uint32_t mc_fmr, mc_fcr;
	volatile uint32_t val;
	struct timeval time_begin, time_curr;
	uint32_t save_CPSR;
		
	/*check frequence value. Must be in range of 1kHz - 55MHz*/
	if(SAM7_frequence < 1 || SAM7_frequence > 55000)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"no valid SAM7_frequence %d kHz\n",LPC_frequence);
		return 2;
	}

	if(SAM7_frequence > 30000)
	{
		clk_wait_mode = 1; // 3 cycles
		num_clkTicksPerMicroSec = ((SAM7_frequence) / 666) + 1;
	}
	else
	{
		clk_wait_mode = 0; // 2 cycles
		num_clkTicksPerMicroSec = ((SAM7_frequence) / 666) + 1;
	}
	/*setup flash mode register */
	mc_fmr	= SAM7_MC_FMR__FMCN(num_clkTicksPerMicroSec)	// Flash microsecond cycle number
		| SAM7_MC_FMR__FWS(clk_wait_mode)		// Flash wait state
		| SAM7_MC_FMR__NEBP(0);				// Erase page before progamming
	/*setup flash command register*/
	mc_fcr	= SAM7_MC_FCR__KEY
		| SAM7_MC_FCR__PAGEN(pageNumber)
		| SAM7_MC_FCR__FCMD(SAM7_MC_FCR__FCMD_WP);
	
	ws = getWorkSpace();
	/*we need the work space and the address start must be at a 128 byte boundary*/
	if( (ws != NULL && (addr & 0x7F) == 0 ))
	{
		/*check if we are able to load the dcc target prog*/
		if ( useWorkspace(WORKSPACE_ALGO_FLASH_ATMEL, addr, 8*1024) == 0 )
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
			gettimeofday(&time_curr, NULL);
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
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 0x4D524140);// "@ARM" = 40 41 52 4D
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, mc_fmr); // mode
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, mc_fcr); // cmd
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, addr); // addr
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 1); // len
			
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
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"ack timeout\n");
					break;
				}
			}

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
				return 3;
			}
			/*write one erased data word*/
			jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
			val = 0xFFFFffff;
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, val);
			
			/*delay by reading DCC status*/
			gettimeofday( &time_begin, NULL);
			for(;;)
			{
				val =jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
				if(val & 0x2)
					break;
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 500msec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				+ (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 500 ) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"write timeout\n");
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
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing fin!(0x%X)\n",val);
				return 4; /*fail*/
			}
			/*OK*/
			return 0;
		}
	}
	/*no way to use the jtag interface directly*/
	IF_DBG(DBG_LEVEL_GDB_ARM_ERROR)
		fputc('?',stdout);
	return 1; /*fail*/
}





