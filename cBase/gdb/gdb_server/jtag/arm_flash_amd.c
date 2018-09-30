/*
 * arm_flash_amd.c
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


/*
 * AMD flash
 * program half word
 */
int prog_flash_amd8(uint32_t base, uint32_t addr, int maxSize, uint8_t *data)
{
	struct memMap *ws;
	uint8_t *bPtr,*tmpbPtr;
	int block_len;
	int block_cnt;
	int len, timeout;
	struct timeval startTime, stopTime;
	volatile uint32_t val;
	uint32_t save_CPSR;
	
	if(*data != 0xFF)
	{
		
		ws = getWorkSpace();
		if(download_faster || (ws != NULL && (addr & 0x3) == 0 ))
		{
			if(ws != NULL)
				block_len = PAGE_SIZE;	// = 4096 Bytes
			else
				block_len = SMALL_PAGE_SIZE;	// = 1024 Bytes

			if(maxSize < block_len)
				block_len = maxSize;

			/*do we have our own work space*/
			if(ws != NULL && (addr & 0x3) == 0 )
			{
				bPtr = data;
				/*if we reach a lot of FF within this block we should stop there*/
				for(block_cnt=1; block_cnt<block_len;block_cnt++)
				{
					bPtr++;
					if(*bPtr == 0xFF)
					{
						tmpbPtr = bPtr;
						/*how many 0xFF do follow*/
						for(len=1;len<(block_len-block_cnt);len++)
						{
							tmpbPtr++;
							if(*tmpbPtr != 0xFF)
								break;
						}
						if(len > MIN_DCC_DATA_SIZE)
						{
							block_len = block_cnt;
							break;
						}
						else
						{
							bPtr = --tmpbPtr;
							block_cnt += len - 1;
						}
					}
				}
				/*convert byte length -block_len- into word length -block_cnt- */
				block_cnt = block_len/(sizeof(uint32_t)/sizeof(uint8_t)); 
				// so block_cnt * 4 is the size in bytes
				if(block_cnt == 0)
					block_cnt = 1;
				/*check if we are able to load the dcc target prog*/
				if (useWorkspace(WORKSPACE_ALGO_FLASH_AMD_8_L, addr, block_cnt*4) == 0 )
				{
					save_CPSR = CPU.CPSR;
					CPU.CPSR = 0xd3; // IRQ and FRQ disabled; Supervisor mode; ARM - State

					IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
						fputc('#',stdout);
					/*DCC control*/
					val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
					if(val & 0x2) // read out data from target
						jtag_arm_IceRT_RegRead_Once(5);
					jtag_arm_ClearAnyBreakPoint();
					jtag_arm_RunProgram(ws->baseAddr + ws->memBufferType.Workspace.offset);
					jtag_arm_enterMonitorMode();
	
					/*write data to target*/
					jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
					do{
						val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
					}while(val & 1);
					jtag_arm_IceRT_RegWrite(5, 0x4D524140);// "@ARM" = 40 41 52 4D
					jtag_arm_IceRT_RegWrite(5, base);
					jtag_arm_IceRT_RegWrite(5, addr);
					jtag_arm_IceRT_RegWrite(5, block_cnt);

					/*read "ack!" = 61 63 6b 21 */
					val = jtag_arm_IceRT_RegRead(5); 
					if(val != 0x216b6361)
					{
						dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing ACK\n");
						jtag_arm_Mointor2DebugMode();
						while(jtag_arm_PollDbgState() == 0)
							;
						/*restore regs*/
						CPU.CPSR = save_CPSR;
						goto fl_write_normal;
					}
					/*write data*/
					do{
						jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
						gettimeofday((&startTime), NULL);
						do {
							val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
							gettimeofday((&stopTime), NULL);
							timeout = (stopTime.tv_sec - startTime.tv_sec) * 1000
								+ (stopTime.tv_usec - startTime.tv_usec) / 1000;
							if( timeout > 500) // 0.5 sec max
							{
								dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"timeout\n");
								jtag_arm_Mointor2DebugMode();
								while(jtag_arm_PollDbgState() == 0)
									;
								/*restore regs*/
								CPU.CPSR = save_CPSR;
								return 0; // fail
							}
						} while (val & 1);
						timeout = 10;

						val = *((uint32_t *)data);
						data+=4; // inc 4 bytes
						jtag_arm_IceRT_RegWrite(5, val);
						block_cnt--;
					}while(block_cnt);
			
					/*read "fin!" = 66 69 6e 21 */
					val = jtag_arm_IceRT_RegRead(5); 
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
						return 0; // fail
					}
				}
				else
					goto fl_write_normal;
			}
			else if(ws != NULL && (addr & 0x3) != 0 )
			{
				/*try to align*/
				block_len = 1;
				goto fl_write_normal;
			}
			else
			{
				bPtr = data;
				/*if we reach a FF within this block we should stop there*/
				for(block_cnt=1; block_cnt<block_len;block_cnt++)
				{
					bPtr++;
					if(*bPtr == 0xFF)
					{
						block_len = block_cnt;
						break;
					}
				}
fl_write_normal:
				IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
					fputc('*',stdout);
				if(jt_amdflashProgByte_faster(base, addr, data, block_len, download_wait))
					return 0; /*fail*/
			}
			/*next addr (faster)*/
			return block_len;
		}
		else
		{
			IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
				fputc('-',stdout);
			if(jt_amdflashProgByte(base, addr, *data, download_wait))
				return 0; /*fail*/
		}
	}
	else // we have seen a 0xFF so we should skip any following 0xFF's too
	{
		block_len = maxSize/sizeof(uint8_t);
		bPtr = data;
		for(block_cnt=1; block_cnt<block_len;block_cnt++)
		{
			bPtr++;
			if(*bPtr != 0xFF)
				break;
		}
		return block_cnt*sizeof(uint8_t);
	}
	/*next addr*/
	return 1;
}

/*
 * AMD flash
 * program half word
 */
int prog_flash_amd16(uint32_t base, uint32_t addr, int maxSize, uint16_t *data)
{
	struct memMap *ws;
	uint16_t *hwPtr,*tmphwPtr;
	int block_len;
	int block_cnt;
	int len, timeout;
	struct timeval startTime, stopTime;
	volatile uint32_t val;
	uint32_t save_CPSR;
		
	if(*data != 0xFFFF)
	{
		
		ws = getWorkSpace();
		if(download_faster || (ws != NULL && (addr & 0x3) == 0 ))
		{
			if(ws != NULL)
				block_len = PAGE_SIZE/sizeof(uint16_t);	// = 2048 halfwords
			else
				block_len = SMALL_PAGE_SIZE/sizeof(uint16_t);	// = 512 halfwords

			if(maxSize < block_len*2)
				block_len = maxSize/sizeof(uint16_t);

			/*do we have our own work space*/
			if(ws != NULL && (addr & 0x3) == 0 )
			{				
				hwPtr = data;
				/*if we reach a lot of FFFF within this block we should stop there*/
				for(block_cnt=1; block_cnt<block_len;block_cnt++)
				{
					hwPtr++;
					if(*hwPtr == 0xFFFF)
					{
						tmphwPtr = hwPtr;
						/*how many 0xFFFF do follow*/
						for(len=1;len<(block_len-block_cnt);len++)
						{
							tmphwPtr++;
							if(*tmphwPtr != 0xFFFF)
								break;
						}
						if(len > MIN_DCC_DATA_SIZE)
						{
							block_len = block_cnt;
							break;
						}
						else
						{
							hwPtr = --tmphwPtr;
							block_cnt += len - 1;
						}
					}
				}

				/*convert halfword length -block_len- into word length -block_cnt- */
				block_cnt = block_len/(sizeof(uint32_t)/sizeof(uint16_t)); 
				// so block_cnt * 4 is the size in bytes
				if(block_cnt == 0)
					block_cnt = 1;
				/*check if we are able to load the dcc target prog*/
				if ( useWorkspace(WORKSPACE_ALGO_FLASH_AMD_16_L, addr, block_cnt*4) == 0 )
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
					jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, base);
					jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, addr);
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
						goto fl_write_normal;
					}

					/*write data*/
					do{
						jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
						gettimeofday((&startTime), NULL);
						do {
							val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
							gettimeofday((&stopTime), NULL);
							timeout = (stopTime.tv_sec - startTime.tv_sec) * 1000
								+ (stopTime.tv_usec - startTime.tv_usec) / 1000;
							if( timeout > 500) // 0.5 sec max
							{
								dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"timeout\n");
								jtag_arm_Mointor2DebugMode();
								while(jtag_arm_PollDbgState() == 0)
									;
								/*restore regs*/
								CPU.CPSR = save_CPSR;
								return 0; // fail
							}
						} while (val & 1);
						val = *((uint32_t *)data);
						data+=2; // inc two halfwords
						jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, val);
						block_cnt--;
					}while(block_cnt);
			
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
						return 0; // fail
					}
				}
				else
				{
					goto fl_write_normal;
				}
			}
			else if(ws != NULL && (addr & 0x3) != 0 )
			{
				/*try to align*/
				block_len = 1;
				goto fl_write_normal;
			}
			else
			{
				hwPtr = data;
				/*if we reach a FFFF within this block we should stop there*/
				for(block_cnt=1; block_cnt<block_len;block_cnt++)
				{
					hwPtr++;
					if(*hwPtr == 0xFFFF)
					{
						block_len = block_cnt;
						break;
					}
				}
fl_write_normal:
				IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
					fputc('*',stdout);
				if(jt_amdflashProgHalfword_faster(base, addr, data, block_len, download_wait))
					return 0; /*fail*/
			}
			/*next addr (faster)*/
			return block_len*sizeof(uint16_t);
		}
		else
		{
			IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
				fputc('-',stdout);
			if(jt_amdflashProgHalfword(base, addr, *data, download_wait))
				return 0; /*fail*/
		}
	}
	else // we have seen a 0xFFFF so we should skip any following 0xFFFF's too
	{
		block_len = maxSize/sizeof(uint16_t);
		hwPtr = data;
		for(block_cnt=1; block_cnt<block_len;block_cnt++)
		{
			hwPtr++;
			if(*hwPtr != 0xFFFF)
				break;
		}
		return block_cnt*sizeof(uint16_t);
	}
	/*next addr*/
	return 2;
}

/*
 * AMD flash
 * program word
 */
int prog_flash_amd32(uint32_t base, uint32_t addr, int maxSize, uint32_t *data)
{
	struct memMap *ws;
	uint32_t *wPtr,*tmpwPtr;
	int block_len;
	int block_cnt;
	int len, timeout;
	struct timeval startTime, stopTime;
	volatile uint32_t val;
	uint32_t save_CPSR;
			
	if(*data != 0xffffFFFF)
	{
		
		ws = getWorkSpace();
		if(download_faster || (ws != NULL && (addr & 0x3) == 0 ))
		{
			if(ws != NULL)
				block_len = PAGE_SIZE/sizeof(uint32_t);	// = 1024 words
			else
				block_len = SMALL_PAGE_SIZE/sizeof(uint32_t);	// = 256 words

			if(maxSize < block_len*4)
				block_len = maxSize/sizeof(uint32_t);

			/*do we have our own work space*/
			if(ws != NULL && (addr & 0x3) == 0 )
			{
				wPtr = data;
				/*if we reach a lot of ffffFFFF within this block we should stop there*/
				for(block_cnt=1; block_cnt<block_len;block_cnt++)
				{
					wPtr++;
					if(*wPtr == 0xffffFFFF)
					{
						tmpwPtr = wPtr;
						/*how many 0xffffFFFF do follow*/
						for(len=1;len<(block_len-block_cnt);len++)
						{
							tmpwPtr++;
							if(*tmpwPtr != 0xffffFFFF)
								break;
						}
						if(len > MIN_DCC_DATA_SIZE)
						{
							block_len = block_cnt;
							break;
						}
						else
						{
							wPtr = --tmpwPtr;
							block_cnt += len - 1;
						}
					}
				}
				/*convert word length -block_len- into word length -block_cnt- */
				block_cnt = block_len; 
				// so block_cnt * 4 is the size in bytes
				if(block_cnt == 0)
					block_cnt = 1;
				/*check if we are able to load the dcc target prog*/
				if ( useWorkspace(WORKSPACE_ALGO_FLASH_AMD_32_L, addr, block_cnt*4) == 0 )
				{
					save_CPSR = CPU.CPSR;
					CPU.CPSR = 0xd3; // IRQ and FRQ disabled; Supervisor mode; ARM - State
					IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
						fputc('#',stdout);
					/*DCC control*/
					val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
					if(val & 0x2) // read out data from target
						jtag_arm_IceRT_RegRead_Once(5);
					jtag_arm_ClearAnyBreakPoint();
					jtag_arm_RunProgram(ws->baseAddr + ws->memBufferType.Workspace.offset);
					jtag_arm_enterMonitorMode();
	
					/*write data to target*/
					jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
					do{
						val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
					}while(val & 1);
					jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, 0x4D524140);// "@ARM" = 40 41 52 4D
					jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, base);
					jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, addr);
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
						goto fl_write_normal;
					}
					/*write data*/
					do{
						jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
						gettimeofday((&startTime), NULL);
						do {
							val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
							gettimeofday((&stopTime), NULL);
							timeout = (stopTime.tv_sec - startTime.tv_sec) * 1000
								+ (stopTime.tv_usec - startTime.tv_usec) / 1000;
							if( timeout > 500) // 0.5 sec max
							{
								dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"timeout\n");
								jtag_arm_Mointor2DebugMode();
								while(jtag_arm_PollDbgState() == 0)
									;
								/*restore regs*/
								CPU.CPSR = save_CPSR;
								return 0; // fail
							}
						} while (val & 1);
						timeout = 10;

						val = *((uint32_t *)data);
						data++; // inc words
						jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, val);
						block_cnt--;
					}while(block_cnt);
			
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
						return 0; // fail
					}
				}
				else
					goto fl_write_normal;
			}
			else if(ws != NULL && (addr & 0x3) != 0 )
			{
				/*try to align*/
				block_len = 1;
				goto fl_write_normal;
			}
			else
			{
				wPtr = data;
				/*if we reach a FFFF within this block we should stop there*/
				for(block_cnt=1; block_cnt<block_len;block_cnt++)
				{
					wPtr++;
					if(*wPtr == 0xffffFFFF)
					{
						block_len = block_cnt;
						break;
					}
				}
fl_write_normal:
				IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
					fputc('*',stdout);
				if(jt_amdflashProgWord_faster(base, addr, data, block_len, download_wait))
					return 0; /*fail*/
			}
			/*next addr (faster)*/
			return block_len*sizeof(uint32_t);
		}
		else
		{
			IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
				fputc('-',stdout);
			if(jt_amdflashProgWord(base, addr, *data, download_wait))
				return 0; /*fail*/
		}
	}
	else // we have seen a 0xffffFFFF so we should skip any following 0xffffFFFF's too
	{
		block_len = maxSize/sizeof(uint32_t);
		wPtr = data;
		for(block_cnt=1; block_cnt<block_len;block_cnt++)
		{
			wPtr++;
			if(*wPtr != 0xffffFFFF)
				break;
		}
		return block_cnt*sizeof(uint32_t);
	}
	/*next addr*/
	return 4;
}


