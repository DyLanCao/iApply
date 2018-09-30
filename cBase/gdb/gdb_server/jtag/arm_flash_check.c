/*
 * arm_gdbstub_check.c
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
 * The CRC 32 polynominal is
 *   32   26   23   22  16   12   11   10   8   7   5   4   2   1   0
 *  X  + X  + X  + X  +X  + X  + X  + X  + X + X + X + X + X + X + X 
 *
 *  3  3322 2222  2222 1111  1111 11
 *  2  1098 7654  3210 9876  5432 1098  7654 3210
 *
 *  1  0000 0100  1100 0001  0000 1101  1011 0111
 * 
 *  0x104C10DB7
 *
 * The CRC 16 polynominal is
 *   16   12   5   0
 *  x  + x  + x + x
 *
 *  1 1111 11
 *  6 5432 1098 7654 3210
 *
 *  1 0001 0000 0010 0001
 *
 *  0x11021
 */

#define CRC32_POLY 0x04C10DB7
#define CRC16_POLY 0x1021

uint32_t arm_checksum_crc32(uint32_t *block, int bit_len)
{
	uint32_t val,next=0;
	int bit_32,i;
	
	/*initial val is 0*/
	val = 0;
	
	for(i=0;i<bit_len;i++)
	{
		/*remember bit[32] for later use*/
		if(val & 0x80000000uL)
			bit_32 = 1;
		else
			bit_32 = 0;
		/*do left shift*/
		val <<= 1;
		/*do we have the next bit*/
		if((i & 0x1F) == 0) // (i % 32) == 0
		{
			/*next bit is highes from block*/
			next = *block++;
		}
		/*insert next bit*/
		if(next & 0x80000000uL)
			val |= 1;
		next <<= 1;

		/*now the polynominal division*/
		if(bit_32)
			val ^= CRC32_POLY;
	}
	return val;
}

uint32_t arm_checksum_crc16(uint16_t *block, int bit_len)
{
	uint32_t val,cnt;
	uint32_t ret_val,next=0;
	int i;

	/*initial val is 0*/
	val = 0;
	cnt = 0;
	
	for(i=0;i<bit_len;i++)
	{
		/*do left shift*/
		val <<= 1;
		/*do we have the next bit*/
		if((i & 0xF) == 0) // (i % 16) == 0
		{
			/*next bit is highes from block*/
			next = *block++;
		}
		/*insert next bit*/
		if(next & 0x8000)
			val |= 1;
		next <<= 1;

		/*now the polynominal division*/
		if(val & 0x10000)
			val ^= CRC32_POLY;
		else
			cnt ++;
	}
	cnt <<= 16;
	ret_val = cnt | val;
	return ret_val;
}



/*
 * Do a CRC32 and a CRC16 at both the host and the target side
 * 
 * return 0 if equal
 * 	  1 if different
 * 	  2 if error with the protocol 
 * 	  3 if workspace not present or unusable
 */
int gdb_check_memory_block(uint32_t addr, uint32_t word_len, uint32_t *buf)
{
	struct memMap *ws;
	int bit_len;
	uint32_t result;
	volatile uint32_t val;
	uint32_t save_CPSR;
	struct timeval time_begin, time_curr;

	ws = getWorkSpace();
	if(ws != NULL)
	{
		/*check if we are able to load the dcc target prog*/
		if ( useWorkspace(WORKSPACE_ALGO_CHECK, addr, word_len*4) == 0 )
		{
			save_CPSR = CPU.CPSR;
			CPU.CPSR = 0xd3; // IRQ and FRQ disabled; Supervisor mode; ARM - State
			IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
				fputc('?',stdout);
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
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 0x4D524140);// "@ARM" = 40 41 52 4D
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, addr);
			bit_len = 32 * word_len;
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, bit_len);
			
			/*calculate the result that we expect*/
			result = arm_checksum_crc32(buf, bit_len);
			
			/*read "ack!" = 61 63 6b 21 */
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_DATA); 
			if(val != 0x216b6361)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing ACK\n");
err_return:				
				jtag_arm_Mointor2DebugMode();
				while(jtag_arm_PollDbgState() == 0)
					;
				/*restore regs*/
				CPU.CPSR = save_CPSR;
				return 2;
			}

			/*receice response*/
			gettimeofday( &time_begin, NULL);
			for(;;)
			{
				val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
				if(val & 0x2) // read out data from target
					break;
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 1 sec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				+ (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 1000) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"(crc32) timeout\n");
					goto err_return;
				}
			}

			/*read out respose*/
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_DATA);
			if(val != result) // crc32 telling us that it is not equal
				goto not_equal;

			/*calculate the result that we expect*/
			result = arm_checksum_crc16((uint16_t *)buf, bit_len);
			
			/*receice response*/
			gettimeofday( &time_begin, NULL);
			for(;;)
			{
				val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
				if(val & 0x2) // read out data from target
					break;
				gettimeofday(&time_curr, NULL);
				/* timeout if more than 1 sec*/
				if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
				+ (time_curr.tv_usec - time_begin.tv_usec)/1000)  > 1000) 
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"(crc16) timeout\n");
					goto err_return;
				}
			}

			/*read out respose*/
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_DATA);
			if(val != result) // crc16 telling us that it is not equal
			{
not_equal:
				jtag_arm_Mointor2DebugMode();
				while(jtag_arm_PollDbgState() == 0)
					;
					
				/*restore regs*/
				CPU.CPSR = save_CPSR;
				return 1;
			}
			
			/*read "fin!" = 66 69 6e 21 */
			val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_DATA); 
			if(val != 0x216e6966)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing fin!\n");
				goto err_return;
			}

			/*switch back to debug mode*/
			//jtag_arm_PutAnyBreakPoint();
			jtag_arm_Mointor2DebugMode();
			while(jtag_arm_PollDbgState() == 0)
				;
			
			/*restore regs*/
			CPU.CPSR = save_CPSR;
			return 0;
		}
		else
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"workspace not usable\n");
		}
	}
	else
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"no workspace\n");
	}
	IF_DBG(DBG_LEVEL_GDB_ARM_ERROR)
		fputc('E',stdout);
	return 3;
}




