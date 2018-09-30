/*
 * arm_gdbstub_break.c
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
#include <string.h>
#include <sys/types.h>

#include <ctype.h>
#include <unistd.h>

#include "dbg_msg.h"
#include "jt_arm.h"
#include "jt_flash.h"
#include "arm_gdbstub.h"

int force_hardware_breakpoint = 0;

struct breakpointContainer breakpointList = {
	0,	//	int numberOfHWBkptEntrys;	
	0,	//	int numberOfSWBkptEntrys;
	0,	//	int numberOfWatchEntrys;
	NULL,	//	struct breakpointEntry *firstHWBkptEntry;
	NULL,	//	struct breakpointEntry *firstSWBkptEntry;
	NULL	//	struct breakpointEntry *firstWatchEntry;
};

/*
 * support function to insert a new Addr to the Berakpoint List
 *
 * Since ICE-RT supports only two watchpoints
 * we can have one of the three solutions:
 *              2 HWBkpt and nither SWBkpt nor Watch
 * alternative	2 Watch and nither HWBkpt nor SWBkpt
 * alternative  unlimited SWBkpt and either one HWBkpt or one SWBkpt
 *
 * Parameter watchORbreak - tells us if it is a watchpoint (=1) or a breakpoint (=0)
 * Parameter len - tells us if it is an ARM (=4) or a Thumb (=2) instruction to break on
 * Parameter addr - is the addess to break on
 *
 * returns 0 on success
 *         1 on Failure
 *         -1 if requeset not supported
 */
int InsertBreakpoint(uint32_t addr, int len, int watchORbreak)
{
	int breakType = 0; // 0 -> HW; 1 -> SW
	struct breakpointEntry *bpEptr, *newBpEptr;
	struct memMap * mm;
	
	addr &= 0xFFFFfffeuL; // make sure Bit 0 set to zero
	/*decide which kind - watch or breakpoint*/
	if(watchORbreak == 0)
	{
		/*at startup we might like to run up to function Main before adding any other breakpoints*/
		if(symbolMain.breakIsActive && symbolMain.state == SYM_PRESENT)
		{
			if(addr != symbolMain.addr)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"Not Main 0x%8.8X\n",addr);
				return 0; // but we tell it's OK
			}
		}
			
		/*check if breakpoint address is already on the HW List*/
		if(breakpointList.numberOfHWBkptEntrys > 0)
		{
			bpEptr = breakpointList.firstHWBkptEntry;
			do {
				if(bpEptr->len == 4) //stored break is ARM
				{
					if(bpEptr->addr == (addr & 0xFFFFfffcuL)) // does address matches
					{
						if(len == 4)
							return 0; // already on the List
						else
							return 1; // wrong 
					}
				}
				else //stored break is Thumb
				{
					if(bpEptr->addr == (addr & 0xFFFFfffeuL)) // does address matches
					{
						if(len == 2)
							return 0; // already on the List
						else
							return 1; // wrong 
					}
				}

				bpEptr = bpEptr->nextBkptEntry;
			} while (bpEptr);
		}
		/*check if breakpoint address is already on the SW List*/
		if(breakpointList.numberOfSWBkptEntrys > 0)
		{
			bpEptr = breakpointList.firstSWBkptEntry;
			do {
				if(bpEptr->len == 4) //stored break is ARM
				{
					if(bpEptr->addr == (addr & 0xFFFFfffcuL)) // does address matches
					{
						if(len == 4)
							return 0; // already on the List
						else
							return 1; // wrong 
					}
				}
				else //stored break is Thumb
				{
					if(bpEptr->addr == (addr & 0xFFFFfffeuL)) // does address matches
					{
						if(len == 2)
							return 0; // already on the List
						else
							return 1; // wrong 
					}
				}

				bpEptr = bpEptr->nextBkptEntry;
			} while (bpEptr);
		}
		/*till now addr is not yet on the List so decide if it should be a HW or SW break*/
		mm = findMemMapOfAddr(addr);
		if(mm == NULL)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"No MemMap for 0x%8.8X\n",addr);
			return 1;
		}
		if(mm->type == MMAP_T_FLASH || mm->type == MMAP_T_APPLICATION_FLASH || mm->type == MMAP_T_ROM)
		{
			/*must be HW break*/
			breakType = 0;
		}
		else if(mm->type == MMAP_T_RAM)
		{
			/*can be SW break*/
			breakType = 1;
			if(force_hardware_breakpoint)
				breakType = 0;
			else if(symbolMain.breakIsActive && symbolMain.state == SYM_PRESENT)
				breakType = 0;
		}
		else
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"Wrong MemMap (type %d) for 0x%8.8X\n",mm->type,addr);
			return 1;
		}
		/*now that we know the Type we check if it is possible with the two ICE-RT Watchpoints we have*/
		if(breakType == 0) // -> HW break
		{
			int sum;
				
			sum = breakpointList.numberOfHWBkptEntrys
			    + breakpointList.numberOfWatchEntrys;
			
			// any SW Breakpoint count as one
			if(breakpointList.numberOfSWBkptEntrys > 0)
				sum++;

			if(sum > 1)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"can't add HW break. List full\n");
				return 1;
			}
			
		}
		else // breakType == 1 -> SW break
		{
			// did we need an extra ICE-RT Watchpoint ?
			if(breakpointList.numberOfSWBkptEntrys == 0)
			{
				int sum;
				
				sum = breakpointList.numberOfHWBkptEntrys
				    + breakpointList.numberOfWatchEntrys;
				if(sum > 1)
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"can't add SW break. List full\n");
					return 1;
				}
			}

		}
		/*till now it is possibel to add a new breakpoint, so let's allocte space for it*/
		newBpEptr = (struct breakpointEntry *) malloc(sizeof(struct breakpointEntry));
		
		if(newBpEptr == NULL)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"NO more Memory for Break\n");
			return 1;
		}
		/*fill common stuff*/
		newBpEptr->nextBkptEntry = NULL;
		/*it is an ARM or Thumb breakpoint*/
		if(len == 4)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Insert @ 0x%8.8X  ARM ", (int)(addr & 0xFFFFfffcuL));
			newBpEptr->len = 4;
			newBpEptr->addr = addr & 0xFFFFfffcuL;
		}
		else
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Insert @ 0x%8.8X THUMB ", (int)(addr & 0xFFFFfffeuL));
			newBpEptr->len = 2;
			newBpEptr->addr = addr & 0xFFFFfffeuL;
		}
		
		/*insert to corresponding List*/
		if(breakType == 0) // insert HW break
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"HW break\n");
			if(breakpointList.numberOfHWBkptEntrys == 0)
			{
				breakpointList.firstHWBkptEntry = newBpEptr;
				breakpointList.numberOfHWBkptEntrys = 1;
			}
			else
			{
				bpEptr = breakpointList.firstHWBkptEntry;
				while(bpEptr->nextBkptEntry != NULL)
					bpEptr = bpEptr->nextBkptEntry;
				bpEptr->nextBkptEntry = newBpEptr;
				breakpointList.numberOfHWBkptEntrys++;
			}
		}
		else // insert SW break
		{
			int page_idx;
			uint32_t offset;

			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Soft break\n");
			if(breakpointList.numberOfSWBkptEntrys == 0)
			{
				breakpointList.firstSWBkptEntry = newBpEptr;
				breakpointList.numberOfSWBkptEntrys = 1;
			}
			else
			{
				bpEptr = breakpointList.firstSWBkptEntry;
				while(bpEptr->nextBkptEntry != NULL)
					bpEptr = bpEptr->nextBkptEntry;
				bpEptr->nextBkptEntry = newBpEptr;
				breakpointList.numberOfSWBkptEntrys++;
			}
			/*collect real target instruction*/
#if 0
			if(len == 4)
				newBpEptr->instr = jtag_arm_ReadWord(addr);
			else
				newBpEptr->instr = jtag_arm_ReadHalfword(addr);
#else
			offset = addr - mm->baseAddr;
			page_idx = (offset)/(RAM_PAGE_SIZE);
			
			if(len == 4)
			{
				if(offset & 3)
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"unaligned ARM instruction at 0x%X\n",addr);
				/*is it already inside of written RAM MMAP*/
				if(   bit_test(mm->memBufferType.Ram.writeByteBitmap, offset)
				   && bit_test(mm->memBufferType.Ram.writeByteBitmap, offset+1)
				   && bit_test(mm->memBufferType.Ram.writeByteBitmap, offset+2)
				   && bit_test(mm->memBufferType.Ram.writeByteBitmap, offset+3)
				  )
					newBpEptr->instr = mm->memBuffer.Word[offset/4];
				/*.. or we have read it before*/
				else if(bit_test(mm->memBufferType.Ram.readPageBitmap, page_idx))
					newBpEptr->instr = mm->memBuffer.Word[offset/4];
				else
					newBpEptr->instr = jtag_arm_ReadWord(addr);
			}
			else
			{
				if(offset & 1)
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"unaligned Thumb instruction at 0x%X\n",addr);
				/*is it already inside of written RAM MMAP*/
				if(   bit_test(mm->memBufferType.Ram.writeByteBitmap, offset)
				   && bit_test(mm->memBufferType.Ram.writeByteBitmap, offset+1)
				  )
					newBpEptr->instr = mm->memBuffer.HalfWord[offset/2] & 0xFFFFuL;
				/*.. or we have read it before*/
				else if(bit_test(mm->memBufferType.Ram.readPageBitmap, page_idx))
					newBpEptr->instr = mm->memBuffer.HalfWord[offset/2] & 0xFFFFuL;
				else
					newBpEptr->instr = jtag_arm_ReadHalfword(addr);
			}
#endif
		}			
	}
	else
	{
		/*it is a watchpoint*/
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"watchpoints not yet supported\n");
		return -1; // not yet supported send ""
	}
	return 0;
}

/*
 * support function to remove a watch or breakpoint if it is on its List
 * If a SW Break will be removed, its original instruction will be also restored back to RAM
 * 
 * Parameter watchORbreak - tells us if it is a watchpoint (=1) or a breakpoint (=0)
 * Parameter len - tells ARM (=4) or Thumb (=2) - still ignored
 * Parameter addr - is the addess of the watch or break
 *
 * returns 0 on success or -1 if nothig happened
 */
int RemoveBreakpoint(uint32_t addr, int len, int watchORbreak)
{
	struct breakpointEntry *bpEptr, *next;
	
	addr &= 0xFFFFfffeuL; // make sure Bit 0 set to zero
	/*decide which kind - watch or breakpoint*/
	if(watchORbreak == 0)
	{
		/*we finish after first match*/
		if(breakpointList.numberOfHWBkptEntrys > 0)
		{
			bpEptr = breakpointList.firstHWBkptEntry;
			if(bpEptr->addr == addr ) // does address matches
			{
				if(bpEptr->len != len)
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"warning: remove command len %d differ from stored %d",len,bpEptr->len);
				/*remove from List*/
				breakpointList.firstHWBkptEntry = bpEptr->nextBkptEntry;
				free(bpEptr);
				breakpointList.numberOfHWBkptEntrys--;
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"HW Break removed\n");
				return 0;
			}
			while (bpEptr->nextBkptEntry)
			{
				next = bpEptr->nextBkptEntry;
				if(next->addr == addr ) // does next address matches
				{
					/*remove next Entry*/
					bpEptr->nextBkptEntry = next->nextBkptEntry;
					free(next);
					breakpointList.numberOfHWBkptEntrys--;
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"HW Break removed\n");
					return 0;
				}
				bpEptr = next;
			}
		}
		if(breakpointList.numberOfSWBkptEntrys > 0)
		{
			bpEptr = breakpointList.firstSWBkptEntry;
			if(bpEptr->addr == addr ) // does address matches
			{
				if(bpEptr->len != len)
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"warning: remove command len %d differ from stored %d",len,bpEptr->len);
				/*remove from List*/
				breakpointList.firstSWBkptEntry = bpEptr->nextBkptEntry;
				/*restore real intruction back to ram*/
				gdb_write_mem( &(bpEptr->instr), addr, bpEptr->len);
				/*XXX since we are reading data at insert time directly from target we must store it now*/
				//gdb_writeback_Ram(); 
				free(bpEptr);
				breakpointList.numberOfSWBkptEntrys--;
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Soft Break removed\n");
				return 0;
			}
			while (bpEptr->nextBkptEntry)
			{
				next = bpEptr->nextBkptEntry;
				if(next->addr == addr ) // does next address matches
				{
					/*remove next Entry*/
					bpEptr->nextBkptEntry = next->nextBkptEntry;
					/*restore real intruction back to ram*/
					gdb_write_mem( &(next->instr), addr, next->len);
					/*XXX since we are reading data at insert time directly from target we must store it now*/
					//gdb_writeback_Ram();
					free(next);
					breakpointList.numberOfSWBkptEntrys--;
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Soft Break removed\n");
					return 0;
				}
				bpEptr = next;
			}
		}
	}
	else
	{
		/*it is a watchpoint*/
		if(breakpointList.numberOfWatchEntrys > 0)
		{
			bpEptr = breakpointList.firstWatchEntry;
			if(bpEptr->addr == addr ) // does address matches
			{
				if(bpEptr->len != len)
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"warning: remove command len %d differ from stored %d",len,bpEptr->len);
				/*remove from List*/
				breakpointList.firstWatchEntry = bpEptr->nextBkptEntry;
				free(bpEptr);
				breakpointList.numberOfWatchEntrys--;
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Watch removed\n");
				return 0;
			}
			while (bpEptr->nextBkptEntry)
			{
				next = bpEptr->nextBkptEntry;
				if(next->addr == addr ) // does address matches
				{
					/*remove this Entry*/
					bpEptr->nextBkptEntry = next->nextBkptEntry;
					free(next);
					breakpointList.numberOfWatchEntrys--;
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Watch removed\n");
					return 0;
				}
				bpEptr = next;
			}
		}
	}
	return -1; // nothing was removed
}

/*
 *
 */
int RemoveAllBreakpoints(void)
{
	struct breakpointEntry *bpEptr, *next;

	dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"remove all breakpoints\n");
	if(breakpointList.numberOfHWBkptEntrys > 0)
	{
		bpEptr = breakpointList.firstHWBkptEntry;
		do {
			/*remove from List*/
			next = bpEptr->nextBkptEntry;
			free(bpEptr);
			bpEptr = next;
		}while (bpEptr);
		breakpointList.numberOfHWBkptEntrys = 0;
		breakpointList.firstHWBkptEntry = NULL;
	}
	if(breakpointList.numberOfSWBkptEntrys > 0)
	{
		bpEptr = breakpointList.firstSWBkptEntry;
		do {
			/*remove from List*/
			next = bpEptr->nextBkptEntry;
			gdb_write_mem( &(bpEptr->instr), bpEptr->addr, bpEptr->len);
			free(bpEptr);
			bpEptr = next;
		}while (bpEptr);
		breakpointList.numberOfSWBkptEntrys = 0;
		breakpointList.firstSWBkptEntry = NULL;
		/*since we are reading data at insert time directly from target we must store it now*/
		gdb_writeback_Ram();
	}
	if(breakpointList.numberOfWatchEntrys > 0)
	{
		bpEptr = breakpointList.firstWatchEntry;
		do {
			/*remove from List*/
			next = bpEptr->nextBkptEntry;
			free(bpEptr);
			bpEptr = next;
		}while (bpEptr);
		breakpointList.numberOfWatchEntrys = 0;
		breakpointList.firstWatchEntry = NULL;
	}
	return 0;
}

/*
 *
 */
int isAddrOnBreakpointList(uint32_t addr)
{
	struct breakpointEntry *bpEptr;

	addr &= 0xFFFFfffeuL; // make sure Bit 0 set to zero
	if(breakpointList.numberOfHWBkptEntrys > 0)
	{
		bpEptr = breakpointList.firstHWBkptEntry;
		do {
			/*check if on List*/
			if(bpEptr->addr == addr)
				return 1;
			bpEptr = bpEptr->nextBkptEntry;
		}while (bpEptr);
	}
	if(breakpointList.numberOfSWBkptEntrys > 0)
	{
		bpEptr = breakpointList.firstSWBkptEntry;
		do {
			/*check if on List*/
			if(bpEptr->addr == addr)
				return 1;
			bpEptr = bpEptr->nextBkptEntry;
		}while (bpEptr);
	}
	return 0;
}


/*
 *
 *
 */
int gdbSetupJtagICE_RTbreakpoint(void)
{
	int watch_reg_num = 0;
	struct breakpointEntry *bpEptr;

	/*first set all HW breaks*/
	if(breakpointList.numberOfHWBkptEntrys > 0)
	{
		bpEptr = breakpointList.firstHWBkptEntry;
		do {
			if(watch_reg_num == 0)
				jtag_arm_PutHWBreakPoint0(bpEptr->addr);
			else if(watch_reg_num == 1)
				jtag_arm_PutHWBreakPoint1(bpEptr->addr);
			else
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"Too many Breakpoints\n");
			
			watch_reg_num++;
			bpEptr = bpEptr->nextBkptEntry;
		}while (bpEptr);
	}
	/*then set the SW breaks*/
	if(breakpointList.numberOfSWBkptEntrys > 0)
	{
		if(watch_reg_num == 0)
			jtag_arm_PutSWBreakPoint0();
		else if(watch_reg_num == 1)
			jtag_arm_PutSWBreakPoint1();
		else
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"Too many Breakpoints. Not able to add SW Break\n");
			
		watch_reg_num++;		
	}
	/*at last the Watches*/
	//XXX not yet
	return 0;
}

/*
 *
 */
int gdbWriteSoftwareBreakToRAM(void)
{
	struct breakpointEntry *bpEptr;
	uint32_t break_instr;	
	
	/*check if it is possible to add any SW break*/
	if(breakpointList.numberOfHWBkptEntrys /*+ breakpointList.numberOfWatchEntrys*/ >= 2)
		return 1;
	
	/*if there are SW breaks*/
	if(breakpointList.numberOfSWBkptEntrys > 0)
	{
		/*make sure that they are stored in RAM*/
		bpEptr = breakpointList.firstSWBkptEntry;
		do {
			if(bpEptr->len == 4) // ARM
			{
				break_instr = 0xB710B710uL;
				gdb_write_mem( &break_instr, bpEptr->addr, 4);
			}
			else // THUMB
			{
				break_instr = 0xB710uL;
				gdb_write_mem( &break_instr, bpEptr->addr, 2);
			}
			bpEptr = bpEptr->nextBkptEntry;
		}while (bpEptr);

		/*since the function is called after a previously gdb_writeback_Ram() we have to repeat this here*/
		gdb_writeback_Ram();
	}

	return 0;	
}

/*
 *
 */
uint32_t gdbLockupThumbInstr(uint32_t addr)
{
	struct memMap * mm;
	uint32_t *buf, val;
	uint32_t page_idx, offset;
	struct breakpointEntry *bpEptr;

	mm = findMemMapOfAddr(addr);
	if(mm != NULL)
	{
		if(mm->type == MMAP_T_FLASH || mm->type == MMAP_T_APPLICATION_FLASH || mm->type == MMAP_T_ROM)
		{
			buf = (uint32_t *) gdb_read_mem(addr, 2);
			val = *buf & 0xFFFFuL;
			return val;
		}
		else if(mm->type == MMAP_T_RAM)
		{
			/* lookup "real" instruction - depending on SW breakpoint*/
			if(breakpointList.numberOfSWBkptEntrys > 0)
			{
				bpEptr = breakpointList.firstSWBkptEntry;
				do {
					/*check if on List*/
					if(bpEptr->addr == addr)
					{
						val = bpEptr->instr & 0xFFFFuL;
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"using real instr 0x%4.4X\n",val);
						return val;
					}
					bpEptr = bpEptr->nextBkptEntry;
				}while (bpEptr);
			}			
			else
			{
				// all possible modifications to the ram are already written back
				// so it it enouth to make a lockup at the read buffer
				// position at memory map buffer
				page_idx = (addr - mm->baseAddr)/(RAM_PAGE_SIZE);
				if(bit_test(mm->memBufferType.Ram.readPageBitmap, page_idx))
				{
					// there is still a valid copy of the instruction inside of our buffer
					// so we use it
					offset = addr - mm->baseAddr;
					buf = (uint32_t *) &(mm->memBuffer.Byte[offset]);
					val = *buf & 0xFFFFuL;
					return val;
				}
				// else - we do not cache the instr. maybe we visit it it only once
			}

		}
	}
	return jtag_arm_ReadHalfword(addr);
}
	
/*
 *
 */
uint32_t gdbLockupArmInstr(uint32_t addr)
{
	struct memMap * mm;
	uint32_t *buf, val;
	uint32_t page_idx, offset;
	struct breakpointEntry *bpEptr;

	mm = findMemMapOfAddr(addr);
	if(mm != NULL)
	{
		if(mm->type == MMAP_T_FLASH || mm->type == MMAP_T_APPLICATION_FLASH || mm->type == MMAP_T_ROM)
		{
			buf = (uint32_t *) gdb_read_mem(addr, 4);
			val = *buf;
			return val;
		}
		else if(mm->type == MMAP_T_RAM)
		{
			/* lookup "real" instruction - depending on SW breakpoint*/
			if(breakpointList.numberOfSWBkptEntrys > 0)
			{
				bpEptr = breakpointList.firstSWBkptEntry;
				do {
					/*check if on List*/
					if(bpEptr->addr == addr)
					{
						val = bpEptr->instr;
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"using real instr 0x%8.8X\n",val);
						return val;
					}
					bpEptr = bpEptr->nextBkptEntry;
				}while (bpEptr);
			}
			else
			{
				// all possible modifications to the ram are already written back
				// so it it enouth to make a lockup at the read buffer
				// position at memory map buffer
				page_idx = (addr - mm->baseAddr)/(RAM_PAGE_SIZE);
				if(bit_test(mm->memBufferType.Ram.readPageBitmap, page_idx))
				{
					// there is still a valid copy of the instruction inside of our buffer
					// so we use it
					offset = addr - mm->baseAddr;
					buf = (uint32_t *) &(mm->memBuffer.Byte[offset]);
					val = *buf;
					return val;
				}
				// else - we do not cache the instr. maybe we visit it it only once
			}
		}
	}
	return jtag_arm_ReadWord(addr);
}



