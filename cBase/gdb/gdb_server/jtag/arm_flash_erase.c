/*
 * arm_gdbstub_mem_flash_erase.c
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

int forceInvalMemWhileEraseingFlash = 0;

/*
 * Callback function to erase a sector segment
 */
int eraseFlashCB_sector(int currAddr, int cnt, struct context *contextSrc)
{
	struct flashCBContext *context = (struct flashCBContext *)contextSrc;
	struct memMap * mm;
	struct cmdCallbackEntry *cbEntry;
	struct sector	*secList;
	int idx,i;
	uint32_t offset;
	struct gdbSprintfBuf *msg_buf;

	if(context == NULL)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"missing context\n");
		return -1;
	}
	mm = context->whichMemMap;
	
	/*make sure we are still in debug state since we need the traget CPU*/
	if ((ice_state.is_debugrequest & 1) == 1)
	{
		secList = mm->memBufferType.Flash.sectorList;
		if(secList == NULL) //should not happen -- paranoia test
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"missing sector list");
			return -1;
		}

		/*do we are going to send messages to the gdb console*/
		if(fake_continue_mode)
			msg_buf = allocateGdbSprintfBuf(BUFMAX);
		else
			msg_buf = NULL;
		
		idx = context->currentSectorId;
		offset = 0;
		for(i=0;i<idx;i++)
			offset += secList[i].size;
		
		/*make sure currAddr is realy within currentSectorId*/
		while( (uint32_t) currAddr >=  mm->baseAddr + offset + secList[idx].size)
		{
			/*correct index and retry*/
			(context->currentSectorId)++;
			idx++;
			offset = 0;
			for(i=0;i<idx;i++)
				offset += secList[i].size;
			if(context->lastSectorId < mm->memBufferType.Flash.numberOfSectors - 1)
				(context->lastSectorId)++;
		}

		/*Check offset alignment*/
		switch(mm->busSize & 0xFF)
		{
		case 8:
			if(offset & 0x7FF) /*2K offset*/
			{
				gdbPrintf(ACTION_NON, 0, msg_buf,"offset failure\n");
				offset &= 0xffff800;
			}
			break;
		case 16:
			if(offset & 0xFFF) /*4K offset*/
			{
				gdbPrintf(ACTION_NON, 0, msg_buf,"offset failure\n");
				offset &= 0xffffF000;
			}
			break;
		case 32:
		case 128:	// pseudo size for Philips
		case 164:	// pseudo size for ST
			if(offset & 0x1FFF) /*8K offset*/
			{
				gdbPrintf(ACTION_NON, 0, msg_buf,"offset failure\n");
				offset &= 0xffffE000;
			}
			break;
		case 132:	// pseudo size for Atmel
			if(offset & 0x7F) /*128 Byte offset*/
			{
				gdbPrintf(ACTION_NON, 0, msg_buf,"offset failure\n");
				offset &= 0xffffFF80;
			}
			break;
		default:;
		}
	
		if(context->quiet_cnt == 0)
			context->verbose |= INFO_VERBOSE;
		if(++context->quiet_cnt == 10)
			context->quiet_cnt = 0;
			
		if((context->verbose & MORE_VERBOSE) == MORE_VERBOSE)
			gdbPrintf( ACTION_NON
				   , 0
				   , msg_buf
				   ,"erase seg %d (%d Bytes) @ Addr 0x%8.8X\t"
				   ,idx,secList[idx].size
				   ,mm->baseAddr + offset);
		else if (context->verbose)
			gdbPrintf( ACTION_NON
				   ,0
				   , msg_buf
				   ,"erase seg %d @ Addr 0x%8.8X\t"
				   ,idx
				   ,mm->baseAddr + offset);
		
		context->verbose &= ~INFO_VERBOSE;
		
		/*erase sector*/
		if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_INTEL)
		{
			switch(mm->busSize & 0xFF)
			{
			case 8:
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"8 bit\t");
				if(secList[idx].lock)
					jt_intelflashUnlockSectorByte(mm->baseAddr + offset, mm->baseAddr + offset);
				jt_intelflashEraseSectorByte(mm->baseAddr + offset, mm->baseAddr + offset); 
				break;
			case 16:
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"16 bit\t");
				switch(mm->memBufferType.Flash.chipsPerBus & 0xF)
				{
				case 1: 
					if(secList[idx].lock)
						jt_intelflashUnlockSectorHalfword(mm->baseAddr + offset, mm->baseAddr + offset);
					jt_intelflashEraseSectorHalfword(mm->baseAddr + offset, mm->baseAddr + offset); 
					break;
				case 2: 
					if(secList[idx].lock)
						jt_intelflashUnlockSectorHalfword_dual(mm->baseAddr + offset, mm->baseAddr + offset);
					jt_intelflashEraseSectorHalfword_dual(mm->baseAddr + offset, mm->baseAddr + offset); 
					break;
				default: 
					gdbPrintf(ACTION_ERASE_FAIL, 1, msg_buf,"%d Flash Chips per 16 Bit Bus not supported\n",mm->memBufferType.Flash.chipsPerBus);
					return 1;
				}
				break;
			case 32:
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"32 bit\t");
				switch(mm->memBufferType.Flash.chipsPerBus & 0xF)
				{
				case 1: 
					if(secList[idx].lock)
						jt_intelflashUnlockSectorWord(mm->baseAddr + offset, mm->baseAddr + offset);
					jt_intelflashEraseSectorWord(mm->baseAddr + offset, mm->baseAddr + offset); 
					break;
				case 2: 
					if(secList[idx].lock)
						jt_intelflashUnlockSectorWord_dual(mm->baseAddr + offset, mm->baseAddr + offset);
					jt_intelflashEraseSectorWord_dual(mm->baseAddr + offset, mm->baseAddr + offset); 
					break;
				case 4: 
					if(secList[idx].lock)
						jt_intelflashUnlockSectorWord_quad(mm->baseAddr + offset, mm->baseAddr + offset);
					jt_intelflashEraseSectorWord_quad(mm->baseAddr + offset, mm->baseAddr + offset); 
					break;
				default: 
					gdbPrintf(ACTION_ERASE_FAIL, 1, msg_buf,"%d Flash Chips per 32 Bit Bus not supported\n",mm->memBufferType.Flash.chipsPerBus);
					return 1;
				}
				break;
			default:	
				gdbPrintf(ACTION_ERASE_FAIL, 1, msg_buf,"Flash Bus wide of %d not supported\n",mm->busSize);
				return 1;
			}
			if(context->verbose)
				gdbPrintf(ACTION_NON, 0, msg_buf, "OK\n");
			else if(!context->quiet_cnt)
				gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[E]\n");
			else
				gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[E]");
		}
		else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_AMD)
		{	
			switch(mm->busSize & 0xFF)
			{
			case 8:
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"8 bit\t");
				if(jt_amdflashEraseSectorByte(mm->baseAddr + offset, mm->baseAddr + offset))
				{
					gdbPrintf(ACTION_ERASE_FAIL, 1, msg_buf,"fail\n");
					return 1;
				}
				break;
			case 16:
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"16 bit\t");
				if(jt_amdflashEraseSectorHalfword(mm->baseAddr + offset, mm->baseAddr + offset))
				{
					gdbPrintf(ACTION_ERASE_FAIL, 1, msg_buf,"fail\n");
					return 1;
				}
				break;
			case 32:
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"32 bit\t");
				if(jt_amdflashEraseSectorWord(mm->baseAddr + offset, mm->baseAddr + offset))
				{
					gdbPrintf(ACTION_ERASE_FAIL, 1, msg_buf,"fail\n");
					return 1;
				}
				break;
			default:
				gdbPrintf(ACTION_ERASE_FAIL, 1, msg_buf,"Flash Bus wide of %d not supported\n",mm->busSize);
				return 1; //not supported
			}
			if(context->verbose)
				gdbPrintf(ACTION_NON, 0, msg_buf, "OK\n");
			else if(!context->quiet_cnt)
				gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[E]\n");
			else
				gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[E]");
		}
		else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_PHILIPS && (mm->busSize & 0xFF) == 128)
		{
			uint32_t mask;
			
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"128 bit\t");
			if( idx < mm->memBufferType.Flash.numberOfSectors - 1)
			{
				/*all except the bootsector itself*/
				mask = jt_philipsflashGenMask( idx, idx, mm->length, LPC_boot_sector_enabled);
			
				if(  philipsFlashUnlock( mm->baseAddr + offset, mask) )
				{	
					gdbPrintf(ACTION_ERASE_FAIL, 1, msg_buf, "fail -- bussy no Unlock\n");
					return 1;
				}

				if( philipsFlashEraseAllUnlocked( mm->baseAddr + offset ) )
				{		
					gdbPrintf(ACTION_ERASE_FAIL, 1, msg_buf, "fail -- bussy no Erase\n");
					return 1;
				}
				if( philipsFlashLock(mm->baseAddr + offset, mask) )
				{
					gdbPrintf(ACTION_ERASE_FAIL, 1, msg_buf, "fail -- bussy no Lock\n");
					return 1;				
				}
				gdbPrintf(ACTION_NON, 0, msg_buf, "OK\n");
			}
			else
			{
				/*bootsector*/
				gdbPrintf(ACTION_ERASE_FAIL, 1, msg_buf, "do not try to erase the bootsector, just for fun\n");
				return 1;
			}
			if(context->verbose)
				gdbPrintf(ACTION_NON, 0, msg_buf, "OK\n");
			else if(!context->quiet_cnt)
				gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[E]\n");
			else
				gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[E]");
		}
		else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_ATMEL && (mm->busSize & 0xFF) == 132)
		{
			uint32_t mc_flash_status_reg;
			
			/*unlock region if necesary*/
			mc_flash_status_reg = jtag_arm_ReadWord(0xffffFF68); // read AT91SAM7 Flash Status Register
			if(mc_flash_status_reg & (1<<(secList[idx].region + 16)))
			{
				if( atmelFlashUnlock( mm->baseAddr + offset, idx) )
				{
					gdbPrintf(ACTION_ERASE_FAIL, 1, msg_buf, "fail -- unlock\n");
					return 1;
				}
			}
			if(atmelFlashErase( mm->baseAddr + offset, idx))
			{		
				gdbPrintf(ACTION_ERASE_FAIL, 1, msg_buf, "fail -- Erase\n");
				return 1;
			}
			if(context->verbose)
				gdbPrintf(ACTION_NON, 0, msg_buf, "OK\n");
			else if(!context->quiet_cnt)
				gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[E]\n");
			else
				gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[E]");
		}
		else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_ST && (mm->busSize & 0xFF) == 164)
		{
			/*XXX todo disable write protection */
			if(stFlashErase( mm->baseAddr + offset, idx))
			{		
				gdbPrintf(ACTION_ERASE_FAIL, 1, msg_buf, "fail -- Erase\n");
				return 1;
			}
			if(context->verbose)
				gdbPrintf(ACTION_NON, 0, msg_buf, "OK\n");
			else if(!context->quiet_cnt)
				gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[E]\n");
			else
				gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[E]");
		}
		else
		{
			gettimeofday(&(memMapContainer.memStat.stopTime), NULL);
			memMapContainer.memStat.pending = ACTION_ERASE_FAIL;
			return 1; //not supported
		}

		/*next Addr*/
		currAddr = (int)(mm->baseAddr + offset + secList[idx].size);
		if(forceInvalMemWhileEraseingFlash)
			secList[idx].valid = SECTOR_INVALID;
		else if(secList[idx].valid & SECTOR_FLAG_HAS_DATA)
			secList[idx].valid = SECTOR_WRITE_DURTY_ERASED; // if there was data it is not there
		
		if(context->currentSectorId < context->lastSectorId)
			(context->currentSectorId)++;
		else
		{
			/*last segment*/
			gettimeofday(&(memMapContainer.memStat.stopTime), NULL);
			memMapContainer.memStat.pending = ACTION_ERASE_SUCCEED;
			if(msg_buf)
					inQueueGdbSprintfBuf(msg_buf);
			return 0;
		}
		if(msg_buf)
			inQueueGdbSprintfBuf(msg_buf);
	}
	else
	{
		memMapContainer.memStat.pending = ACTION_ERASE_FAIL;
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"must be in debug state\n");
		return 1;
	}
		
	/*create context of next callback*/
	cbEntry = allocateCmdCallbackEntry(sizeof(struct flashCBContext));
	if(cbEntry == NULL)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"no Mem for new Entry\n");
		return -1;
	}

	inQueueCmdCallbackEntry(
				cbEntry,
				eraseFlashCB_sector,
				currAddr & 0xFFFFfffc,
				cnt,
				contextSrc,
				sizeof(struct flashCBContext)
				);

	return 0;
}

