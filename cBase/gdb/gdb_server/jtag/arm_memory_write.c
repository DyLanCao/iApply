/*
 * arm_memory_write.c
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
 * Writeback the modified data block into the targets memory space.
 */
void gdb_writeback_memory_block(uint32_t addr, uint32_t word_len, uint32_t *buf)
{
	struct memMap *ws;
	volatile uint32_t val;
	uint32_t save_CPSR;

	ws = getWorkSpace();
	/*check if we are able to load the dcc target prog*/
	if( ws != NULL && useWorkspace(WORKSPACE_ALGO_WRITE, addr, word_len*4) == 0 )
	{
		save_CPSR = CPU.CPSR;
		CPU.CPSR = 0xd3; // IRQ and FRQ disabled; Supervisor mode; ARM - State
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
			goto write_normal;
		}
		/*write data*/
		do{
			val = *buf;
			buf++;
			jtag_arm_IceRT_RegWrite(ICERT_REG_DCC_DATA, val);
			word_len--;
		}while(word_len);
			
		/*read "fin!" = 66 69 6e 21 */
		val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_DATA); 
		if(val != 0x216e6966)
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing fin!\n");
		/*switch back to debug mode*/
		//jtag_arm_PutAnyBreakPoint();
		jtag_arm_Mointor2DebugMode();
		while(jtag_arm_PollDbgState() == 0)
			;
		/*restore regs*/
		CPU.CPSR = save_CPSR;
		return;
	}
write_normal:
	jtag_arm_WriteMemoryBuf(addr , word_len, buf );
	return;
}


/*
 * write new data into local buffer.
 * (The real write will be done a little bit later in the writeback phase.)
 *
 */
int gdb_write_mem(int *mem_val, int addr, int length)
{
	struct memMap *mm;
	uint32_t write_len, mem_pos;
	uint32_t u_len= (uint32_t)length;
	uint32_t u_addr = (uint32_t) addr;
	uint8_t *mem_val_byte = (uint8_t *) mem_val;
	
	if(length == 0 || mem_val_byte == NULL) // write at least one Byte
	{
		if(mem_val_byte == NULL)
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"can't write 0x%8.8X length %d (NULL)\n",addr, length);
		else
			//dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"can't write 0x%8.8X length %d (%x)\n",addr, length, *mem_val);
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"can't write 0x%8.8X length %d (?)\n",addr, length);
		return 1; // failure
	}

	do {
		mm = findMemMapOfAddr(u_addr);
	
		if(mm == NULL)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"no map for address 0x%8.8X\n",u_addr);
			return 1; // failure
		}

		/*does length fit in this memory map*/
		/*XXX todo if baseAddr and u_addr are both in high address range this can fail*/
		if( (mm->baseAddr + mm->length) >= (u_addr + u_len) )
			write_len = u_len;
		else
			write_len = (mm->baseAddr + mm->length) - u_addr;
		
		mem_pos = u_addr - mm->baseAddr; // position at memory map buffer

		switch(mm->type)
		{	
			struct sector	*curr_sec;
			int i,j;
			uint32_t size, uloc_addr, uloc_len;

		case MMAP_T_FLASH:
			//search for a sector where the addr is hold
			curr_sec = mm->memBufferType.Flash.sectorList;
			if(curr_sec == NULL) //should not happen -- paranoia test
				return 1; // failure 
			uloc_addr = u_addr;
			uloc_len = write_len;
			size = 0;
			
			for(i=0; i < mm->memBufferType.Flash.numberOfSectors; i++)
			{
				size += curr_sec->size;
				if(uloc_addr < (mm->baseAddr + size))
				{
					if(curr_sec->valid == SECTOR_INVALID)
					{
						// fill Flash sector with FF's -- same as after eraseing the Flash
						for(j=curr_sec->size; j>0; j--)
							mm->memBuffer.Byte[size-j] = 0xFF;
					}
					//fit. So mark it as it having write data.
					curr_sec->valid = SECTOR_WRITE;
					if(uloc_len > (uint32_t)curr_sec->size)
					{
						uloc_addr += curr_sec->size;
						uloc_len -= curr_sec->size;
					}
					else
					{
						uloc_addr += uloc_len;
						uloc_len = 0;
					}
					// end if it's still in this sector -> so it's the last
					if(uloc_addr < (mm->baseAddr + size)) 
						break;
				}
				
				curr_sec++; //next
			}
			// write it into the marked buffer
			bcopy(mem_val_byte, mm->memBuffer.Byte + mem_pos, write_len);
			// mark as durty
			bit_nset(mm->memBufferType.Flash.writeByteBitmap, mem_pos, mem_pos + write_len - 1);
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"@ 0x%8.8X %d Bytes %2.2xh\n",u_addr, write_len, *mem_val_byte);
			break;
		case MMAP_T_RAM:
			//write bytes
			bcopy(mem_val_byte, mm->memBuffer.Byte + mem_pos, write_len);
			// mark as durty
			bit_nset(mm->memBufferType.Ram.writeByteBitmap, mem_pos, mem_pos + write_len - 1);
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"@ 0x%8.8X %d Bytes %2.2xh\n",u_addr, write_len, *mem_val_byte);
			break;
		case MMAP_T_UNREAL:
			bcopy(mem_val_byte, mm->memBuffer.Byte + mem_pos, write_len);
			break;
		case MMAP_T_UNUSED:
		case MMAP_T_APPLICATION_FLASH:
		case MMAP_T_ROM:
		case MMAP_T_SFA:
		case MMAP_T_IO:
		case MMAP_T_WORKSPACE:
		case MMAP_T_CACHE:
		default:
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"wrong map type %d at address 0x%8.8X\n",mm->type ,u_addr);
			return 1; // failure
		}
		/*try next map if possible*/
		u_len -= write_len;
		u_addr += write_len;
	} while(u_len != 0);
	
	return 0; // OK
}

/*
 * Mark all cached RAM - Data is being invalid now.
 */
int gdb_invalidate_Ram_Buffer(void)
{
	struct memMap *m;
	struct memMapHead *mh;

	mh = memMapContainer.activeMap;
	if(mh == NULL || mh->numberOfEntrys == 0)
		return -1;
	
	m = mh->firstMapEntry;
	while(m != NULL)
	{
		if(m->type == MMAP_T_RAM && m->length > 0)
			bzero(m->memBufferType.Ram.readPageBitmap , (m->length/(RAM_PAGE_SIZE*8*sizeof(bitstr_t))+1) * sizeof(bitstr_t));
		m = m->nextMap;
	}

	return 0;
}

/*
 *
 */
int gdb_invalidate_Rom_Buffer(void)
{
	struct memMap *m;
	struct memMapHead *mh;

	mh = memMapContainer.activeMap;
	if(mh == NULL || mh->numberOfEntrys == 0)
		return -1;
	
	m = mh->firstMapEntry;
	while(m != NULL)
	{
		if(m->type == MMAP_T_ROM && m->length > 0)
			bzero(m->memBufferType.Ram.readPageBitmap , (m->length/(RAM_PAGE_SIZE*8*sizeof(bitstr_t))+1) * sizeof(bitstr_t));
		m = m->nextMap;
	}

	return 0;
}


/*
 *
 */
int gdb_writeback_Ram(void)
{
	int write_count;
	int pos;
	struct memMap *m, *ws;
	struct memMapHead *mh;
	uint32_t wsAddr;

	mh = memMapContainer.activeMap;
	if(mh == NULL || mh->numberOfEntrys == 0)
		return -1;
	
	ws = getWorkSpace();
	if(ws != NULL && ws->type == MMAP_T_RAM )
		wsAddr = ws->baseAddr +  ws->memBufferType.Workspace.offset;
	else 
		wsAddr = ~0 - WORKSPACE_SEGMENT_SIZE;
	
	m = mh->firstMapEntry;
	while(m != NULL)
	{
		if(m->type == MMAP_T_RAM && m->length > 0)
		{
			/*find first position of a changed value in RAM*/
			bit_ffs(m->memBufferType.Ram.writeByteBitmap, (int)m->length, &pos);

			while(pos >= 0 && pos < (int)m->length)
			{
				/*we do not transfer RAM type workspace now -- we will do this a little bit later*/
				if( ws != NULL && ws->type == MMAP_T_RAM  )
				{
					if(   m->baseAddr + pos >= wsAddr 
					   && m->baseAddr + pos < wsAddr + WORKSPACE_SEGMENT_SIZE)
					{
						pos++;
						goto find_next_pos;
					}
				}
				/*figure out how many Bytes have to be written back*/
				for(write_count=1; (pos + write_count)<(int)m->length; write_count++)
				{
					if( !bit_test(m->memBufferType.Ram.writeByteBitmap , (pos + write_count)))
						break;
					/*except those wich are lying inside of the workspace*/
					if( ws != NULL 
					  && ws->type == MMAP_T_RAM  
					  && m->baseAddr + pos + write_count >= wsAddr 	
					  && m->baseAddr + pos + write_count < wsAddr + WORKSPACE_SEGMENT_SIZE)
							break;
				}
				/*clear those from Bitmap now*/
				bit_nclear(m->memBufferType.Ram.writeByteBitmap, pos, pos + write_count - 1);
				
				/*let's see if there is a byte ahead the halfword boundary*/
				if(pos & 0x1uL)
				{
					jtag_arm_WriteByte(m->baseAddr + pos, m->memBuffer.Byte[pos]);
					pos++;
					write_count--;
				}
				/*check of halfword's before word boundary*/
				if((pos & 0x2uL) == 0x2uL && write_count > 1)
				{
					jtag_arm_WriteHalfword(m->baseAddr + pos, m->memBuffer.HalfWord[pos/2]);
					pos += 2;
					write_count -= 2;
				}
				/*now write back all word alined stuff*/
				if(write_count > 3)
				{
					int howmany_word;

					howmany_word = write_count/4;
					if(howmany_word == 1)
						jtag_arm_WriteWord(m->baseAddr + pos, m->memBuffer.Word[pos/4]);
					else
						gdb_writeback_memory_block(m->baseAddr + pos,howmany_word,&(m->memBuffer.Word[pos/4]));
						//jtag_arm_WriteMemoryBuf
					pos += howmany_word * 4;
					write_count -= howmany_word * 4;
				}
				/*now the remaining halfword*/
				if(write_count > 1)
				{
					jtag_arm_WriteHalfword(m->baseAddr + pos, m->memBuffer.HalfWord[pos/2]);
					pos += 2;
					write_count -= 2;
				}
				/*and the last byte*/
				if(write_count > 0)
				{
					jtag_arm_WriteByte(m->baseAddr + pos, m->memBuffer.Byte[pos]);
					pos++;
					write_count--;
				}
				/*paranoia check*/
				if(write_count != 0)
					dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"shit not all bytes written back\n");
find_next_pos:
				/*search for a next positition within the write bitmap*/
				for(; pos <  (int)m->length; pos++)
				{
					if( bit_test(m->memBufferType.Ram.writeByteBitmap , pos))
						break;
				}
			}
			
		}
		m = m->nextMap;
	}
	
	/*in case we are having a RAM based workspace, this has to be written back too. Also the workspace is free now*/
	if( ws != NULL && ws->type == MMAP_T_RAM)
	{	
		if(  ws->memBufferType.Workspace.state != WORKSPACE_UNTESTED 
		  && ws->memBufferType.Workspace.state != WORKSPACE_BROKEN)
			ws->memBufferType.Workspace.state = WORKSPACE_FREE;
		
		/*find first position of a changed value in RAM-workspace*/
		bit_ffs(ws->memBufferType.Ram.writeByteBitmap, (int)ws->length, &pos);
		while(pos >= 0 && pos < (int)ws->length)
		{
			/*figure out how many Bytes have to be written back*/
			for(write_count=1; (pos + write_count)<(int)ws->length; write_count++)
			{
				if( !bit_test(ws->memBufferType.Ram.writeByteBitmap , (pos + write_count)))
					break;
			}
			/*clear those from Bitmap now*/
			bit_nclear(ws->memBufferType.Ram.writeByteBitmap, pos, pos + write_count - 1);
				
			/*let's see if there is a byte ahead the halfword boundary*/
			if(pos & 0x1uL)
			{
				jtag_arm_WriteByte(ws->baseAddr + pos, ws->memBuffer.Byte[pos]);
				pos++;
				write_count--;
			}
			/*check of halfword's before word boundary*/
			if((pos & 0x2uL) == 0x2uL && write_count > 1)
			{
				jtag_arm_WriteHalfword(ws->baseAddr + pos, ws->memBuffer.HalfWord[pos/2]);
				pos += 2;
				write_count -= 2;
			}
			/*now write back all word alined stuff*/
			if(write_count > 3)
			{
				int howmany_word;

				howmany_word = write_count/4;
				if(howmany_word == 1)
					jtag_arm_WriteWord(ws->baseAddr + pos, ws->memBuffer.Word[pos/4]);
				else
					jtag_arm_WriteMemoryBuf(ws->baseAddr + pos,howmany_word,&(ws->memBuffer.Word[pos/4]));
				pos += howmany_word * 4;
				write_count -= howmany_word * 4;
			}
			/*now the remaining halfword*/
			if(write_count > 1)
			{
				jtag_arm_WriteHalfword(ws->baseAddr + pos, ws->memBuffer.HalfWord[pos/2]);
				pos += 2;
				write_count -= 2;
			}
			/*and the last byte*/
			if(write_count > 0)
			{
				jtag_arm_WriteByte(ws->baseAddr + pos, ws->memBuffer.Byte[pos]);
				pos++;
				write_count--;
			}
			/*paranoia check*/
			if(write_count != 0)
				dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"shit not all workspace bytes written back\n");
			/*search for a next positition within the write bitmap*/
			for(; pos <  (int)ws->length; pos++)
			{
				if( bit_test(ws->memBufferType.Ram.writeByteBitmap , pos))
					break;
			}
		}
	}
	return 0;
}


