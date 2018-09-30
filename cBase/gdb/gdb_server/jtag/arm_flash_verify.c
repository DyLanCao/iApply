/*
 * arm_gdbstub_mem.c
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
 * Callback to verify the memory at the target and the buffered memory at the host
 */
int verifyFlashCB_read_sector(int currAddr, int cnt, struct context *contextSrc)
{	
	struct flashCBContext *context = (struct flashCBContext *)contextSrc;
	struct memMap * mm;
	struct cmdCallbackEntry *cbEntry;
	struct sector	*secList;
	int idx,i;
	char *buf;
	uint32_t offset;
	enum pendingACT act;
	struct gdbSprintfBuf *msg_buf;

	if(context == NULL)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing context\n");
		return -1;
	}
	mm = context->whichMemMap;
	
	/*make sure we are still in debug state since we need the traget CPU*/
	if ((ice_state.is_debugrequest & 1) == 1)
	{
		secList = mm->memBufferType.Flash.sectorList;
		if(secList == NULL) //should not happen -- paranoia test
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"missing sector list");
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
		/*read sector in pages of 512Byte and mark it as holding data*/
		if(cnt == -1)
		{
			/*first page so need a buffer tha can hold the data*/
			buf = (char *)malloc(secList[i].size);
			if(buf == NULL)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"no mem for temp. buffer\n");
				return -1;
			}
			context->temp_buffer = buf;
			cnt = 0;
			
			if((context->verbose & MORE_VERBOSE) == MORE_VERBOSE)
				gdbPrintf( ACTION_NON 
					   ,0 
					   ,msg_buf 
					   ,"reading seg %d (%d Bytes) @ Addr 0x%8.8X\t"
					   ,idx
					   ,secList[idx].size
					   ,mm->baseAddr + offset);
			else if(context->verbose)
				gdbPrintf( ACTION_NON 
					   ,0 
					   ,msg_buf 
					   ,"reading seg %d @ Addr 0x%8.8X\t"
					   ,idx
					   ,mm->baseAddr + offset);
		}
		else
		{
			buf = context->temp_buffer;
		}
		context->verbose &= ~INFO_VERBOSE;
			
		if(secList[idx].size > (PAGE_SIZE*(cnt+1))) //do more pages follow
		{
			// Read Word Memory
			gdb_read_memory_block(mm->baseAddr + offset + (PAGE_SIZE*cnt)
				, PAGE_SIZE / sizeof(uint32_t)
				, (uint32_t *)(buf + PAGE_SIZE*cnt));
			cnt++;
			
			context->quiet_cnt--; //reverse previous incrementation, since more pages following
			
			if(msg_buf && context->verbose)
			{
				snprintf(&msg_buf->string[msg_buf->len] ,BUFMAX - msg_buf->len ,".");
				msg_buf->len += 1;
			}
		}
		else
		{
			/*this is the last page within this sector segment*/
			/* at the atmel the segment size with 128 Byte is quite small
			 * but if we try to read more segmets it might be usefull to
			 * disable the test of the lenght on the workspace*/
			if(secList[idx].size * (context->lastSectorId - idx) > PAGE_SIZE)
				disable_workspace_size_test = 1;
			// Read Word Memory
			gdb_read_memory_block(mm->baseAddr + offset + (PAGE_SIZE*cnt)
				, (secList[idx].size - (PAGE_SIZE*cnt))/sizeof(uint32_t)
				, (uint32_t *)(buf + PAGE_SIZE*cnt));
			disable_workspace_size_test = 0;
		
			switch(secList[idx].valid)
			{
			case SECTOR_INVALID:
				/*copy local buffer to mem map buffer*/
				for(i=0; i<secList[idx].size; i++)
					mm->memBuffer.Byte[offset + i] = buf[i];

				secList[idx].valid = SECTOR_READ_VALID;
				if(context->verbose)
					gdbPrintf( ACTION_NON ,0 ,msg_buf ,"First Time segment is clean\n");
				else if(!context->quiet_cnt)
					gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[R]\n");
				else
					gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[R]");
				break;
			case SECTOR_READ_VALID:
			case SECTOR_READ_ERASED_VALID:
			case SECTOR_WRITE:
			case SECTOR_WRITE_DURTY:
			case SECTOR_WRITE_DURTY_ERASED:
			       /*check if they are equal or not*/
				for(i=0; i<secList[idx].size; i++)
				{
					if(mm->memBuffer.Byte[offset + i] != (uint8_t) buf[i])
					{
						secList[idx].valid = SECTOR_WRITE_DURTY;
						if(context->verbose)
							gdbPrintf( ACTION_NON ,0 ,msg_buf ,"segment is durty @ 0x%8.8X\n",mm->baseAddr + offset + i);
						else if(!context->quiet_cnt)
							gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[D]\n");
						else
							gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[D]");
						break;
					}
				}
				if(i>=secList[idx].size)
				{
					secList[idx].valid = SECTOR_READ_VALID;
					if(context->verbose)
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"segment is clean\n");
					else if(!context->quiet_cnt)
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[V]\n");
					else
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"[V]");
				}
				break;
			case SECTOR_FLAG_RW:
			case SECTOR_FLAG_DURTY:
			case SECTOR_FLAG_ERASED:
			default: /*unknown state*/
				gdbPrintf( ACTION_NON ,1 ,msg_buf ,"unknown internal state of Flash\n");
				free(buf);
				return -1;
			}	
			/*are all data erased*/
			for(i=0; i<secList[idx].size; i++)
			{
				uint8_t tst = (uint8_t) buf[i];
				if(tst != 0xFF)
					break;
			}
			if(i>=secList[idx].size)
				secList[idx].valid |= SECTOR_FLAG_ERASED;

			free(buf);

			/*next Addr*/
			currAddr = (int)(mm->baseAddr + offset + secList[idx].size);
			cnt = -1;
		
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
				if(msg_buf)
					inQueueGdbSprintfBuf(msg_buf);
				return 0;
			}
		}
		IF_DBG( DBG_LEVEL_ALL )
			fflush(stdout);
		if(msg_buf)
			inQueueGdbSprintfBuf(msg_buf);
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
				verifyFlashCB_read_sector,
				currAddr & 0xFFFFfffc,
				cnt,
				contextSrc,
				sizeof(struct flashCBContext)
				);

	return 0;
}


