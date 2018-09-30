/*
 * arm_gdbstub_mem_flash_readchk.c
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

int forceCheckInvalidSector = 0;
/*
 * Callback to start generating a check value at the target and the host
 * if both are equal the memory will be hopefully the same (but this is not always true)
 * -- this function requires some workspace at the target side --
 */
int checkFlashCB_read_sector(int currAddr, int cnt, struct context *contextSrc)
{	
	struct flashCBContext *context = (struct flashCBContext *)contextSrc;
	struct memMap * mm;
	struct cmdCallbackEntry *cbEntry;
	struct sector	*secList;
	int idx,i,result;
	uint32_t offset;
	enum pendingACT act;
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
		while( (uint32_t) currAddr >= mm->baseAddr + offset + secList[idx].size )
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
		
		if(context->quiet_cnt == 0)
			context->verbose |= INFO_VERBOSE;
		if(++context->quiet_cnt == 10)
			context->quiet_cnt = 0;
		if((context->verbose & MORE_VERBOSE) == MORE_VERBOSE)
			gdbPrintf( ACTION_NON 
				   ,0 
				   ,msg_buf 
				   ,"checking seg %d (%d Bytes) @ Addr 0x%8.8X\t"
				   ,idx
				   ,secList[idx].size
				   ,mm->baseAddr + offset);
		else if (context->verbose)
			gdbPrintf( ACTION_NON
				   ,0
				   , msg_buf
				   ,"checking seg %d @ Addr 0x%8.8X\t"
				   ,idx
				   ,mm->baseAddr + offset);

		context->verbose &= ~INFO_VERBOSE;

		/*check sector segment*/
		if( forceCheckInvalidSector || secList[idx].valid != SECTOR_INVALID)
		{
			result = gdb_check_memory_block(mm->baseAddr + offset
						, secList[idx].size/sizeof(uint32_t)
						, (uint32_t *)(&mm->memBuffer.Byte[offset]) );
			if(result == 0)
			{
				/*it is likely that the segment at the target is equal to the mapped data */
				switch(secList[idx].valid)
				{
				case SECTOR_INVALID:
					if(context->verbose)
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"segment is still marked as being invalid\n");
					else if(!context->quiet_cnt)
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[i]\n");
					else
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[i]");
				break;
				case SECTOR_READ_VALID:
				case SECTOR_READ_ERASED_VALID:
				case SECTOR_WRITE:
				case SECTOR_WRITE_DURTY:
				case SECTOR_WRITE_DURTY_ERASED:
			    		secList[idx].valid = SECTOR_READ_VALID;
					if(context->verbose)
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"segment is clean\n");
					else if(!context->quiet_cnt)
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[v]\n");
					else
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[v]");
					break;
				case SECTOR_FLAG_RW:
				case SECTOR_FLAG_DURTY:
				case SECTOR_FLAG_ERASED:
				default: /*unknown state*/
					gdbPrintf( ACTION_NON ,1 ,msg_buf ,"unknown internal state of Flash\n");
					return -1;
				}
			}
			else if(result == 1)
			{
				/*the segment is now known to be durty*/
				secList[idx].valid = SECTOR_WRITE_DURTY;
				if(context->verbose)
					gdbPrintf( ACTION_NON ,0 ,msg_buf ,"segment is durty\n");
				else if(!context->quiet_cnt)
					gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[d]\n");
				else
					gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[d]");
			}
			else if(result == 2)
			{
				/*something failed with the protocol*/
				gdbPrintf( ACTION_NON ,0 ,msg_buf ,"<internal protocol error; abort>\n");
				memMapContainer.memStat.pending = ACTION_ERROR;
				return 1;
			}
			else
			{
				/*wokrspace not present of unusable*/
				gdbPrintf( ACTION_NON ,0 ,msg_buf ,"<broken or missing workspace; abort>\n");
				memMapContainer.memStat.pending = ACTION_ERROR;
				return 1;
			}
		}
		else /* skip checking of invalid segments */
		{
			if(context->verbose)
				gdbPrintf( ACTION_NON ,0 ,msg_buf ,"skiping check; segment is still invalid\n");
			else if(!context->quiet_cnt)
				gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[s]\n");
			else
				gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[s]");
		}
		IF_DBG( DBG_LEVEL_ALL )
			fflush(stdout);
		if(msg_buf)
			inQueueGdbSprintfBuf(msg_buf);
		
		/*next Addr*/
		currAddr = (int)(mm->baseAddr + offset + secList[idx].size);
		
		if(context->currentSectorId < context->lastSectorId)
			(context->currentSectorId)++;
		else
		{
			/*check if at least one segment is still durty*/
			act = ACTION_VERIFY_EQUAL; 
			for(idx = 0; idx <= context->lastSectorId; idx++)
			{
				if((secList[idx].valid & SECTOR_FLAG_DURTY) == SECTOR_FLAG_DURTY)
				{
					act = ACTION_VERIFY_DIFF;
					break;
				}
			}
			gettimeofday(&(memMapContainer.memStat.stopTime), NULL);
			memMapContainer.memStat.pending = act;
			return 0;
		}
	}
	else
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"must be in Debug state\n");
		memMapContainer.memStat.pending = ACTION_ERROR;
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
				checkFlashCB_read_sector,
				currAddr & 0xFFFFfffc,
				cnt,
				contextSrc,
				sizeof(struct flashCBContext)
				);

	return 0;
}

