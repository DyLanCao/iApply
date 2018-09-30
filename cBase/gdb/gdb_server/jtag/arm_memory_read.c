/*
 * arm_memory_read.c
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
void gdb_read_memory_block(uint32_t addr, uint32_t word_len, uint32_t *buf)
{	
	struct memMap *ws;
	volatile uint32_t val;
	uint32_t save_CPSR;

	ws = getWorkSpace();
	/*check if we are able to load the dcc target prog*/
	if(ws != NULL && useWorkspace(WORKSPACE_ALGO_READ, addr, word_len*4) == 0 )
	{
		save_CPSR = CPU.CPSR;
		CPU.CPSR = 0xd3; // IRQ and FRQ disabled; Supervisor mode; ARM - State
		IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
			fputc('+',stdout);
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
		jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, word_len);

		jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_DATA); //send read command
		/*receive response*/
		do{
			if(word_len == 1)
				val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_CONTROL);
			else
				val = jtag_arm_IceRT_RegRead_Once(ICERT_REG_DCC_DATA);
			*buf = val;
			buf++;
			word_len--;
		}while(word_len);
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
		fputc('.',stdout);
	jtag_arm_ReadWordMemory(addr , word_len, buf );
	return;
}


/*
 * Read out memory from target into local buffer if not already done.
 * Return a pointer to the local copy.
 *
 */
int *gdb_read_mem(int addr, int length)
{
	struct memMap *mm;
	int page_idx, sector_idx;
	uint32_t offset;
	
	if(length == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"can't read 0x%8.8X length (zero)\n",addr);
		return NULL;
	}
	/*lockup*/
	mm = findMemMapOfAddr(addr);
	
	if(mm == NULL)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"no read map at address 0x%8.8X\n",addr);
		return NULL; // failure
	}

	/*does length fit in this memory map*/
	if( (mm->baseAddr + mm->length) >= ((uint32_t)addr + (uint32_t)length) )
	{
		offset = (uint32_t)addr - mm->baseAddr;
		switch(mm->type)
		{
			struct sector	*curr_sec;
			uint32_t size, uloc_addr;

		case MMAP_T_FLASH:
		case MMAP_T_APPLICATION_FLASH:
			/*calculate sector index and search for a sector where the addr is hold*/
			curr_sec = mm->memBufferType.Flash.sectorList;
			if(curr_sec == NULL) //should not happen -- paranoia test
				break; 
			uloc_addr = (uint32_t) addr;
			size = 0;
			
			for(sector_idx=0; sector_idx < mm->memBufferType.Flash.numberOfSectors; sector_idx++)
			{
				size += curr_sec->size;
				if(uloc_addr < (mm->baseAddr + size))
				{
					switch(curr_sec->valid)
					{
						char *buf;
						int i;
						
					case SECTOR_WRITE:
					case SECTOR_INVALID: 
						/*read hole sector now and mark it as holding data*/
						buf = (char *)malloc(curr_sec->size);
						if(buf == NULL)
						{
							dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"no mem for temp. buffer\n");
							return NULL;
						}
						//jtag_arm_ReadWordMemory
						gdb_read_memory_block(mm->baseAddr + (size - curr_sec->size) , curr_sec->size/sizeof(uint32_t), (uint32_t *)buf );
						curr_sec->valid |= SECTOR_FLAG_HAS_DATA;
						
						if(curr_sec->valid == SECTOR_WRITE && mm->type == MMAP_T_FLASH) /*parts are changed */
						{
							curr_sec->valid |= SECTOR_FLAG_DURTY;
							/*copy in only data that is not yet modified*/
							for(i=0; i<curr_sec->size; i++)
							{
								if(!bit_test(mm->memBufferType.Flash.writeByteBitmap, size-curr_sec->size + i))
									mm->memBuffer.Byte[size-curr_sec->size + i] = buf[i];
							}

						}
						else
						{
							/*copy local buffer to mem map buffer*/
							for(i=0; i<curr_sec->size; i++)
							{
								mm->memBuffer.Byte[size-curr_sec->size + i] = buf[i];
							}
							if(mm->type == MMAP_T_FLASH)
							{
								/*are all data erased*/
								for(i=0; i<curr_sec->size; i++)
								{
									int tst = buf[i];
									if(tst != 0xFF)
										break;
								}
								if(i>=curr_sec->size)
									curr_sec->valid |= SECTOR_FLAG_ERASED;
							}
						}
						free(buf);
					case SECTOR_READ_VALID: 
					case SECTOR_READ_ERASED_VALID:
						/*sector is already in our buffer and known to be valid*/
					case SECTOR_WRITE_DURTY:
					case SECTOR_WRITE_DURTY_ERASED:
						/*sector is known to be durty but buffer is holding the right data*/
						break;
					case SECTOR_FLAG_RW:
					case SECTOR_FLAG_DURTY:
					case SECTOR_FLAG_ERASED:
					default:
						dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"invalid sector state %d\n",curr_sec->valid);
						return NULL;
					}
					if((uloc_addr + (uint32_t)length) <= (mm->baseAddr + size))
						return (int *) &(mm->memBuffer.Byte[offset]); // unalign intger hu, better we return char *
				}
				
				curr_sec++; //next
			}
			dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"ups missing Flash\n");
			break;
		case MMAP_T_RAM:
		case MMAP_T_ROM:
			/*let us see if we already have read the page*/
			/*The size of each page is  4 * 32 Byte = 128 Byte = 64 Halfwords = 32 Words*/
			page_idx = ((uint32_t)addr - mm->baseAddr)/(RAM_PAGE_SIZE);
			do {
				if(!bit_test(mm->memBufferType.Ram.readPageBitmap, page_idx))
				{
					char buf[RAM_PAGE_SIZE];
					int i;
				
					/*not yet read - so do this now*/
					gdb_read_memory_block(mm->baseAddr + page_idx*RAM_PAGE_SIZE, RAM_PAGE_SIZE/sizeof(uint32_t), (uint32_t *)buf );
	
					/*mark page being in buffer*/
					bit_set(mm->memBufferType.Ram.readPageBitmap, page_idx);
	
					/*copy in only data that is not yet modified*/
					for(i=0; i<(int)RAM_PAGE_SIZE; i++)
					{
						if(mm->type != MMAP_T_RAM || !bit_test(mm->memBufferType.Ram.writeByteBitmap, page_idx*RAM_PAGE_SIZE + i))
							mm->memBuffer.Byte[page_idx*RAM_PAGE_SIZE + i] = buf[i];
					}
				}
				if((uint32_t)addr + (uint32_t)length <= (page_idx + 1) * RAM_PAGE_SIZE + mm->baseAddr )
					return (int *) &(mm->memBuffer.Byte[offset]); // unalign intger hu, better we return char *
				page_idx++;
			}while((uint32_t)page_idx < mm->length/(RAM_PAGE_SIZE));
			dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"ups missing RAM\n");
			break;
		case MMAP_T_UNREAL:
			return (int *) &(mm->memBuffer.Byte[offset]);
		case MMAP_T_UNUSED:
		case MMAP_T_SFA:
		case MMAP_T_IO:
		case MMAP_T_WORKSPACE:
		case MMAP_T_CACHE:
		default:
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"wrong memmap type %d\n",mm->type);
		}
	}
	else
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"read exceeds boundary limit\n");
	return NULL; //failure
}


