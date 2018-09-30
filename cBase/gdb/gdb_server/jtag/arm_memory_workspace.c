/*
 * arm_memory_workspace.c
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

int disable_workspace_size_test = 0;

/*
 *
 *
 */
struct memMap * getWorkSpace(void)
{
	return memMapContainer.workspace;
}

/*
 *
 */
int is_workspace_big_enough (uint32_t length)
{
	if ( length >= WORKSPACE_SEGMENT_SIZE
	     && (uint32_t)dcc_read_size < WORKSPACE_TEXT_SEGMENT_SIZE
	     && (uint32_t)dcc_write_size < WORKSPACE_TEXT_SEGMENT_SIZE
	     && (uint32_t)dcc_check_size < WORKSPACE_TEXT_SEGMENT_SIZE
	     && (uint32_t)dcc_fl_amd8_size < WORKSPACE_TEXT_SEGMENT_SIZE  // stack using
	     && (uint32_t)dcc_fl_amd16_size < WORKSPACE_TEXT_SEGMENT_SIZE // stack using
	     && (uint32_t)dcc_fl_amd32_size < WORKSPACE_TEXT_SEGMENT_SIZE // stack using
	     && (uint32_t)dcc_fl_philips_size < WORKSPACE_SEGMENT_SIZE    // not using stack
	     && (uint32_t)dcc_fl_atmel_size < WORKSPACE_SEGMENT_SIZE    // not using stack
	     && (uint32_t)dcc_fl_st_size < WORKSPACE_SEGMENT_SIZE    // not using stack
	   )
	{
		return 1;
	}
	else
	{
		dbgPrintf (
			DBG_LEVEL_GDB_ARM_ERROR,
			"problem with workspace:\n"
			"workspace size is %d (should >= %d text %d)\n"
			"target size: read(%d) write(%d) check(%d) amd16(%d) lpc(%d) atmel(%d) str(%d)\n"
			,length
			,WORKSPACE_SEGMENT_SIZE
			,WORKSPACE_TEXT_SEGMENT_SIZE
			,dcc_read_size
			,dcc_write_size
			,dcc_check_size
			,dcc_fl_amd16_size
			,dcc_fl_philips_size
			,dcc_fl_atmel_size
			,dcc_fl_st_size
			);
	}
	return 0;
}

/*
 *
 *
 */
void changeWorkSpaceMode( int mode )
{
	struct memMap *m, *m_tmp, *m_maybe;
	struct memMapHead *mh;

	/*anything to do?*/
	if (mode == workspace_mode)
		return;
	
	/*is there already an active MMap*/
	mh = memMapContainer.activeMap;
	if(mh == NULL || mh->numberOfEntrys == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"no active MMAP -- can't change workspace mode\n");
		workspace_mode = mode;
		return;
	}

	/*make sure that there is no pending action*/
	if( (memMapContainer.memStat.pending & ACTION_INPROCESS) == ACTION_INPROCESS )
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"changeWorkSpaceMode() failed setting default -2(pending)\n");
		workspace_mode = -2;
		return;
	}
		
	/*deativate RAM as workspace*/
	if(mode == 0 && memMapContainer.workspace != NULL && memMapContainer.workspace->type == MMAP_T_RAM)
	{
		memMapContainer.workspace = NULL;
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"workspace not active\n");
	}
	/*aktivate Top of RAM as workspace if not an extra workspace is already given*/
	else if(mode == 1 && (memMapContainer.workspace == NULL || memMapContainer.workspace->type == MMAP_T_RAM) )
	{
		/*now we have to locate the top of all ram's*/
		m = mh->firstMapEntry;
		m_tmp = NULL;
		m_maybe = NULL;
		while(m != NULL)
		{
			if(   m->type == MMAP_T_RAM 
			   && m->length >= WORKSPACE_SEGMENT_SIZE 
			   && (m_tmp == NULL || m_tmp->baseAddr < m->baseAddr))
				m_tmp = m;
			m = m->nextMap;
			
			/*check if the work space big enouth to hold .text and stack*/
			if ( m_tmp != NULL && is_workspace_big_enough(m_tmp->length))
				m_maybe = m_tmp;
		}
		if ( m_maybe != NULL)
		{
			memMapContainer.workspace = m_maybe;
			m_maybe->memBufferType.Workspace.state = WORKSPACE_UNTESTED;
			m_maybe->memBufferType.Workspace.info = TOP_OF_RAM_WORKSPACE;
			m_maybe->memBufferType.Workspace.offset = (m_maybe->length & ~(RAM_PAGE_SIZE - 1))- WORKSPACE_SEGMENT_SIZE;
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO
				,"workspace at top of RAM (%d byte @ 0x%8X)\n"
				,WORKSPACE_SEGMENT_SIZE
				,m_maybe->baseAddr+m_maybe->memBufferType.Workspace.offset);
		}
		else
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"no RAM for workspace found\n");
	}
	/*aktivate Bottom of RAM as workspace if not an extra workspace is already given*/
	else if(mode == 2 && (memMapContainer.workspace == NULL || memMapContainer.workspace->type == MMAP_T_RAM) )
	{
		/*now we have to locate the first of all ram's*/
		m = mh->firstMapEntry;
		m_tmp = NULL;
		m_maybe = NULL;
		while(m != NULL && m_tmp != 0)
		{
			if(   m->type == MMAP_T_RAM 
			   && m->length >= WORKSPACE_SEGMENT_SIZE 
			   && (m_tmp == NULL || m_tmp->baseAddr > m->baseAddr))
				m_tmp = m;
			m = m->nextMap;
			/*check if the work space big enouth to hold .text and stack*/
			if ( m_tmp != NULL && is_workspace_big_enough(m_tmp->length) )
				;
			else
				m_tmp = NULL;
		}
		if ( m_tmp != NULL )
		{
			memMapContainer.workspace = m_tmp;
			m_tmp->memBufferType.Workspace.state = WORKSPACE_UNTESTED;
			m_tmp->memBufferType.Workspace.info = BOTTOM_OF_RAM_WORKSPACE;
			m_tmp->memBufferType.Workspace.offset = 0;
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO
				,"workspace at bottom of RAM (%d byte @ 0x%8X)\n"
				,WORKSPACE_SEGMENT_SIZE
				,m_tmp->baseAddr+m_tmp->memBufferType.Workspace.offset);
		}
		else
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"no RAM for workspace found\n");
	}
	else if(mode != 0)
	{
		if(memMapContainer.workspace == NULL || memMapContainer.workspace->type != MMAP_T_WORKSPACE)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"did not set mode %d ( mode is still %d)\n",mode,workspace_mode);
			return;
		}
	}
	if (memMapContainer.workspace != NULL && memMapContainer.workspace->type == MMAP_T_WORKSPACE)
	{
		m_tmp = memMapContainer.workspace;
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO
			,"stand alone workspace (%d byte @ 0x%8X)\n"
			,m_tmp->length
			,m_tmp->baseAddr);
	}
	
	workspace_mode = mode;
	return;
}


/*
 * useWorkspace
 * 	Transfers a given algorithm-program to the workspace of the target machine.
 * 	
 * parameter
 *   alog	: Enum ID of the target algorithm we like to transfer
 *   addr	: Base address of the data we like to modify or check
 *   lenght	: Lenght of the data we like to modify or check
 * 
 * return 0 on sucsess else error number
 * 	  1 algorithm not yet supported
 * 	  2 not usefull to transfer algorithm
 * 	  4 target algorithm has an align failure
 * 	  5 there is no workspace
 */
int useWorkspace(int algo, uint32_t addr, unsigned length)
{
	struct memMap *ws;
	uint32_t baseAddr;
	int page_idx;
	int len;
	unsigned int dcc_start;
	int dcc_size;

	/*set start address and size depending on requested algorithm*/
	switch( algo )
	{
	case WORKSPACE_ALGO_DUMMY:
		dcc_start = dcc_dummy_start;
		dcc_size  = dcc_dummy_size;
		break;
	case WORKSPACE_ALGO_READ:
		dcc_start = dcc_read_start;
		dcc_size  = dcc_read_size;
		break;
	case WORKSPACE_ALGO_WRITE:
		dcc_start = dcc_write_start;
		dcc_size  = dcc_write_size;
		break;
	case WORKSPACE_ALGO_CHECK:
		dcc_start = dcc_check_start;
		dcc_size  = dcc_check_size;
		break;
	case WORKSPACE_ALGO_FLASH_AMD_8_L:
		dcc_start = dcc_fl_amd8_start;
		dcc_size  = dcc_fl_amd8_size;
		break;
	case WORKSPACE_ALGO_FLASH_AMD_16_L:
		dcc_start = dcc_fl_amd16_start;
		dcc_size  = dcc_fl_amd16_size;
		break;
	case WORKSPACE_ALGO_FLASH_AMD_32_L:
		dcc_start = dcc_fl_amd32_start;
		dcc_size  = dcc_fl_amd32_size;
		break;
	case WORKSPACE_ALGO_FLASH_PHILIPS:
		dcc_start = dcc_fl_philips_start;
		dcc_size  = dcc_fl_philips_size;
		break;
	case WORKSPACE_ALGO_FLASH_ATMEL:
		dcc_start = dcc_fl_atmel_start;
		dcc_size  = dcc_fl_atmel_size;
		break;
	case WORKSPACE_ALGO_FLASH_ST:
		dcc_start = dcc_fl_st_start;
		dcc_size  = dcc_fl_st_size;
		break;
	case WORKSPACE_ALGO_FLASH_AMD_8_B:
	case WORKSPACE_ALGO_FLASH_AMD_16_B:
	case WORKSPACE_ALGO_FLASH_AMD_32_B:
	default:
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"algo %d not yet supported\n",algo);
		return 1;
	}
	
	ws = getWorkSpace();
	if( ws != NULL )
	{
		/*check if we have to load the dcc target prog*/
		if((int)ws->memBufferType.Workspace.state ==  algo)
			return 0; /* it's allread loaded nothing more left to do */
			
		/*check if it is reasonable to write data via work space program that is running at the target*/
		if( ( disable_workspace_size_test == 0
		      && ( length < MIN_DCC_DATA_SIZE 
		         || (  algo != WORKSPACE_ALGO_FLASH_ATMEL /*(check skiped because)we must use the the even when data transfer is bigger than real data*/
		            && algo != WORKSPACE_ALGO_CHECK /*(check skiped because) we can't do this check without workspace*/
		            && length < WORKSPACE_SEGMENT_SIZE
		            )
		         )
		    )
		    || (ws->memBufferType.Workspace.state == WORKSPACE_BROKEN)
		  )
		{
			if(ws->memBufferType.Workspace.state == WORKSPACE_BROKEN)
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"workspace broken\n");
			else
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN
					,"skip using workspace data lenght too small %d\n"
					,length);
			return 2;
		}
		/*save RAM workspace data before enter*/
		if(ws->type == MMAP_T_RAM)
		{
			baseAddr = ws->baseAddr + ws->memBufferType.Workspace.offset;

			/*transfering data, which is placed inside of the workspce itselfe did not make any sence*/
			if ( addr >=  baseAddr && addr < baseAddr + WORKSPACE_SEGMENT_SIZE ) // start address is lying inside of forbidden areal
				return 2; // useless
			if ( addr + length >= baseAddr && addr + length < baseAddr + WORKSPACE_SEGMENT_SIZE ) // stop address is lying inside of forbidden areal
				return 2; // useless
			if ( addr <= baseAddr && addr + length > baseAddr + WORKSPACE_SEGMENT_SIZE ) // start and stop address overlapping both forbidden areal
				return 2; // useless
		
			if(   ws->memBufferType.Workspace.state == WORKSPACE_UNTESTED
			   || ws->memBufferType.Workspace.state == WORKSPACE_FREE)
			{
				/*let us see if we already have read the page*/
				/*The size of each page is  4 * 32 Byte = 128 Byte = 64 Halfwords = 32 Words*/
				page_idx = ((uint32_t)baseAddr - ws->baseAddr)/(RAM_PAGE_SIZE);
				do {
					if(!bit_test(ws->memBufferType.Ram.readPageBitmap, page_idx))
					{
						char buf[RAM_PAGE_SIZE];
						int i;
					
						/*not yet read - so do this now*/
						jtag_arm_ReadWordMemory(ws->baseAddr + page_idx*RAM_PAGE_SIZE, RAM_PAGE_SIZE/sizeof(uint32_t), (uint32_t *)buf );
	
						/*mark page being in buffer*/
						bit_set(ws->memBufferType.Ram.readPageBitmap, page_idx);
	
						/*copy in only data that is not yet modified*/
						for(i=0; i<(int)RAM_PAGE_SIZE; i++)
						{
							if( !bit_test(ws->memBufferType.Ram.writeByteBitmap, page_idx*RAM_PAGE_SIZE + i))
								ws->memBuffer.Byte[page_idx*RAM_PAGE_SIZE + i] = buf[i];
						}
						/*now we mark them all as being durty since we like to write the original data back*/
						bit_nset(ws->memBufferType.Ram.writeByteBitmap, page_idx*RAM_PAGE_SIZE, page_idx*RAM_PAGE_SIZE + RAM_PAGE_SIZE - 1);
					}
					if((uint32_t)baseAddr + WORKSPACE_SEGMENT_SIZE <= (page_idx+1) * RAM_PAGE_SIZE + ws->baseAddr )
						break ; // 
					page_idx++;
				}while((uint32_t)page_idx < ws->length/(RAM_PAGE_SIZE));
				if ((uint32_t)page_idx >= ws->length/(RAM_PAGE_SIZE))
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"ups missing RAM end\n");
			}
		}
		else
			baseAddr = ws->baseAddr;
		
		/*check if work space is usable*/
		if(ws->memBufferType.Workspace.state == WORKSPACE_UNTESTED)
		{
			jtag_arm_WriteWord((baseAddr + 4)&0xFFFFfffc,0xdeadbeaf);
			jtag_arm_WriteWord((baseAddr + WORKSPACE_SEGMENT_SIZE - 4)&0xFFFFfffc,0xfaebdaed);
			if(   jtag_arm_ReadWord((baseAddr + 4)&0xFFFFfffc) != 0xdeadbeaf 
			   || jtag_arm_ReadWord((baseAddr + WORKSPACE_SEGMENT_SIZE - 4)&0xFFFFfffc) != 0xfaebdaed)
			{
				ws->memBufferType.Workspace.state = WORKSPACE_BROKEN;
				dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"workspace unsuable\n");
			}
			else
				ws->memBufferType.Workspace.state = WORKSPACE_FREE;
		}
		/*if usable use it*/
		if( ws->memBufferType.Workspace.state != WORKSPACE_BROKEN )
		{
			len = dcc_size / sizeof(uint32_t);
			if(dcc_size & 3)
				len++;
			if(dcc_start & 3)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"align failure\n");
				ws->memBufferType.Workspace.state = WORKSPACE_BROKEN;
				return 4;
			}
			jtag_arm_WriteMemoryBuf(baseAddr,len,(uint32_t *)dcc_start);
			ws->memBufferType.Workspace.state = algo;
			return 0; // OK
		}
	}
	return 5;
}

