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


/********************************/
struct memMapContainer memMapContainer = {0,NULL,-1,NULL,NULL,{0,{0,0},{0,0}}};
int workspace_mode = 0;

/*
 *
 * returns 0 on success else error number
 */
int AllocMemMaps(int total_number_of_MemMaps)
{
	int add_nmbr, i;
	struct memMapHead *mh, *curr_mh, *prev_mh;

	if(memMapContainer.numberOfMemMaps >= total_number_of_MemMaps)
		return 0; // nothing to do
	
	add_nmbr = total_number_of_MemMaps - memMapContainer.numberOfMemMaps;
	mh = (struct memMapHead *) malloc(add_nmbr * sizeof(struct memMapHead));
	if(mh == NULL)
		return ENOMEM;
	bzero(mh,add_nmbr * sizeof(struct memMapHead));

	/*find last MemMapHead*/
	curr_mh = memMapContainer.firstMap;
	prev_mh = NULL;
	while(curr_mh != NULL)
	{
		prev_mh = curr_mh;
		curr_mh = curr_mh->nextHead;
	}

	if(prev_mh == NULL) // it was the first in the container
		memMapContainer.firstMap = mh;
	else
		prev_mh->nextHead = mh;

	/*generete missing list*/
	for(i=1; i<add_nmbr; i++)
	{
		mh->nextHead = &mh[1];
		mh++;
	}
	memMapContainer.numberOfMemMaps = total_number_of_MemMaps;
	return 0;
}


/*
 *
 *
 */
struct memMapHead * searchMemMapHead(int num)
{
	int i;
	struct memMapHead *mh;

	if(num >= memMapContainer.numberOfMemMaps || num < 0)
		return NULL;
	
	mh = memMapContainer.firstMap;
	if(num == 0)
		return mh;
	
	for(i=0; i<num; i++)
		mh = mh->nextHead;
	return mh;
}

/*
 *
 *
 */
int AllocateMemMapEntrys(struct memMapHead * mh, int total_num)
{
	int add_nmbr, i;
	struct memMap *m, *curr_m, *prev_m;

	if(mh == NULL)
		return EINVAL;
	if(mh->numberOfEntrys >= total_num)
		return 0;

	add_nmbr = total_num - mh->numberOfEntrys;
	m = (struct memMap *) malloc(add_nmbr * sizeof(struct memMap));
	if(m == NULL)
		return ENOMEM;
	bzero(m,add_nmbr * sizeof(struct memMap));
	
	/*find last MemMap*/
	curr_m = mh->firstMapEntry;
	prev_m = NULL;
	while(curr_m != NULL)
	{
		prev_m = curr_m;
		curr_m = curr_m->nextMap;
	}

	if(prev_m == NULL) // it was the first in the container
		mh->firstMapEntry = m;
	else
		prev_m->nextMap = m;

	/*generete missing list*/
	for(i=1; i<add_nmbr; i++)
	{
		m->nextMap = &m[1];
		m++;
	}
	mh->numberOfEntrys = total_num;
	return 0;
	
}

/*
 *
 *
 */
struct memMap * searchMemMapEntry(struct memMapHead * mh,int num)
{
	int i;
	struct memMap *m;

	if(mh == NULL || num >= mh->numberOfEntrys || num < 0)
		return NULL;
	
	m = mh->firstMapEntry;
	for(i=0; i<num; i++)
		m = m->nextMap;
	return m;
}

/*
 *
 *
 */
struct memMap * findMemMapOfAddr(uint32_t addr)
{
	struct memMap *m;
	struct memMapHead *mh;

	mh = memMapContainer.activeMap;
	if(mh == NULL || mh->numberOfEntrys == 0)
		return NULL;
	
	m = mh->firstMapEntry;
	while(m != NULL)
	{
		if(addr >= m->baseAddr)
		{
			if (m->baseAddr & 0x80000000)
			{
				if( (addr - 0x80000000uL) < (m->baseAddr - 0x80000000uL) + m->length )
					return m;
			}
			else if(addr < (m->baseAddr + m->length) )
				return m;
		}
		m = m->nextMap;
	}
	return NULL;
}

/*
 *
 *
 */
int updateMemMap(struct memMapHead * mh, int num 
		,enum memMapType type, uint32_t busSize, uint32_t baseAddr, uint32_t length)
{
	int i;
	struct memMap *m, *curr_m;

	/*check if usable request data (of mh and num) -- and that an Entrys exist*/
	if(mh == NULL || mh->firstMapEntry == NULL)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"can't update Memory Map no Entry\n");
		return EINVAL;
	}
	if( num >= mh->numberOfEntrys )
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"can't update Memory Map not enought Entrys left\n");
		return EINVAL;
	}
	if( num < 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"can't update Memory Map number is negative\n");
		return EINVAL;
	}
	if(memMapContainer.activeNumber == num)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"can't update Memory Map while it is in use\n");
		return EINVAL;
	}
	
	/*find entry*/
	m = mh->firstMapEntry;
	for(i=0; i<num; i++)
		m = m->nextMap;
	
	if(type != MMAP_T_UNUSED)
	{
		/*now check for overlapping*/
		curr_m = mh->firstMapEntry;
		for(i=0; ; i++)
		{
			/*curr_m has entry with data*/
			if(curr_m->type != MMAP_T_UNUSED && curr_m != m)
			{
				/*overlap ?*/
				if(curr_m->baseAddr <= baseAddr)
				{
					if((curr_m->baseAddr + (curr_m->length - 1))>= baseAddr )
					{	
						dbgPrintf(DBG_LEVEL_GDB_ARM_WARN
							,"can't update Memory Map (overlapping)%d - %d\n"
							,num,i);
						return EINVAL;
					}
				}
				else // curr_m->baseAddr > baseAddr
				{
					if( (baseAddr + (length - 1))>= curr_m->baseAddr)
					{
						dbgPrintf(DBG_LEVEL_GDB_ARM_WARN
							,"can't update Memory Map (overlapping)%d - %d\n"
							,num,i);
						return EINVAL;
					}
				}
				
			}
			if(i>=mh->numberOfEntrys || curr_m->nextMap == NULL)
				break;
			curr_m = curr_m->nextMap;
		}
	}
	/*till now no overlapping detected so fill in data*/
	if(m->type != MMAP_T_UNUSED)
	{
		// changing is not yet tested - so garbage might remains in memory
		if(m->type == MMAP_T_FLASH || m->type == MMAP_T_APPLICATION_FLASH)
		{
			m->memBufferType.Flash.algo		= FLASH_ALGORITHEM_UNKOWN;
			m->memBufferType.Flash.numberOfSectors	= 0;
			if(m->memBufferType.Flash.sectorList != NULL)
			{
				free(m->memBufferType.Flash.sectorList);
				m->memBufferType.Flash.sectorList	= NULL;
			}
		}
	}
	m->type		= type;
	m->busSize	= busSize;
	m->baseAddr	= baseAddr;
	m->length	= length;
	return 0;
}

/*
 *
 *
 */
static int detectFlash(struct memMap *m)
{
	struct sector *sector_info, *curr_sec, *prev_sec;
	int offset = 0;
	int num, size, ret_val;
	uint32_t adr;
		
	/*on every FLASH -> see which kind of Flash it is and allocate its sector info*/
	adr = m->baseAddr;
	prev_sec = NULL;
	sector_info = NULL;
	m->memBufferType.Flash.chipsPerBus = 0;
	/* try Intel */
	do {
		switch(m->busSize & 0xFF)
		{
		case 8:
			ret_val = jt_intelflashGetInfoByte(adr , &sector_info);
			break;
		case 16:	
			ret_val = jt_intelflashGetInfoHalfword(adr , &sector_info);
			break;
		case 32:	// intel 32 not yet supported
		case 164:	// maybe ST
		case 132:	// maybe Atmel
		case 128:	// maybe Philips
			ret_val = 0;
			break;
		default:
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"Flash Bus with of %d not supported\n",m->busSize);
			return 1; //not supported
		}
		if(ret_val == 0 || sector_info == NULL)
			break;
		/*find out how may sectors are found*/
		num = 0;
		size = 0;
		curr_sec = sector_info;
		while( curr_sec->size != 0 )
		{
			size += curr_sec->size;
			curr_sec++;
			num++;
		}
		m->memBufferType.Flash.numberOfSectors += num;
		m->memBufferType.Flash.algo = FLASH_ALGORITHEM_INTEL;
		/*test if prevoius length is equal to the new one*/
		if(m->memBufferType.Flash.chipsPerBus == 0)
			m->memBufferType.Flash.chipsPerBus = ret_val;
		else if (m->memBufferType.Flash.chipsPerBus != ret_val)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"mixing of differen number of Flash Chips is not supported. You must define an addititonal memmory map.\n");
			return 1; //not supported
		}
		offset = size;
		if(prev_sec != NULL)
		{
			/*reallocate space for the new and the prev sector_info*/
			curr_sec = (struct sector *) malloc((m->memBufferType.Flash.numberOfSectors + 1) * sizeof(struct sector));
			if(curr_sec == NULL)
			{
				printf("No Mem");
				exit(EX_OSERR);
			}
			bcopy(prev_sec,curr_sec, (m->memBufferType.Flash.numberOfSectors - num) * sizeof(struct sector));
			free(prev_sec);
			prev_sec = curr_sec;
			curr_sec += (m->memBufferType.Flash.numberOfSectors - num);
			bcopy(sector_info, curr_sec, num * sizeof(struct sector));
			curr_sec += num;
			bzero(curr_sec, sizeof(struct sector));
			free(sector_info);
			m->memBufferType.Flash.sectorList = prev_sec;
		}
		else
		{
			m->memBufferType.Flash.sectorList = sector_info;
			prev_sec = sector_info;
		}
		adr += offset;
	} while(adr < m->baseAddr + m->length);

	if(m->memBufferType.Flash.algo == FLASH_ALGORITHEM_INTEL)
		return 0;
	
	/* try AMD */
	adr = m->baseAddr;
	offset = 0;
	prev_sec = NULL;
	do {
		switch(m->busSize & 0xFF)
		{
		case 8:
			ret_val = jt_amdflashGetInfoByte(adr , &sector_info);
			break;
		case 16:	
			ret_val = jt_amdflashGetInfoHalfword(adr , &sector_info);
			break;
		case 32:
			ret_val = jt_amdflashGetInfoWord(adr , &sector_info);
			break;
		case 164:	// maybe ST
		case 132:	// maybe Atmel
		case 128:	// maybe Philips
			ret_val = 0;
			break;
		default:
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"Flash Bus with of %d not supported\n",m->busSize);
			return 1; //not supported
		}
		if(ret_val == 0 || sector_info == NULL)
			break;
		/*find out how may sectors are found*/
		num = 0;
		size = 0;
		curr_sec = sector_info;
		while( curr_sec->size != 0 )
		{
			size += curr_sec->size;
			curr_sec++;
			num++;
		}
		m->memBufferType.Flash.numberOfSectors += num;
		m->memBufferType.Flash.algo = FLASH_ALGORITHEM_AMD;
		/*test if prevoius length is equal to the new one*/
		if(m->memBufferType.Flash.chipsPerBus == 0)
			m->memBufferType.Flash.chipsPerBus = ret_val;
		else if (m->memBufferType.Flash.chipsPerBus != ret_val)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"mixing of differen number of Flash Chips is not supported. You must define an addititonal memmory map.\n");
			return 1; //not supported
		}
		offset = size;
		if(prev_sec != NULL)
		{
			/*reallocate space for the new and the prev sector_info*/
			curr_sec = (struct sector *) malloc((m->memBufferType.Flash.numberOfSectors + 1) * sizeof(struct sector));
			if(curr_sec == NULL)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"No Mem");
				exit(EX_OSERR);
			}
			bcopy(prev_sec,curr_sec, (m->memBufferType.Flash.numberOfSectors - num) * sizeof(struct sector));
			free(prev_sec);
			prev_sec = curr_sec;
			curr_sec += (m->memBufferType.Flash.numberOfSectors - num);
			bcopy(sector_info, curr_sec, num * sizeof(struct sector));
			curr_sec += num;
			bzero(curr_sec, sizeof(struct sector));
			free(sector_info);
			m->memBufferType.Flash.sectorList = prev_sec;
		}
		else
		{
			m->memBufferType.Flash.sectorList = sector_info;
			prev_sec = sector_info;
		}
		adr += offset;
	} while(adr < m->baseAddr + m->length);
	
	if(m->memBufferType.Flash.algo == FLASH_ALGORITHEM_AMD)
		return 0;

	if( (m->busSize & 0xFF) == 164 )// ST
	{
		ret_val = jt_stflashGetInfo( m->baseAddr , m->length , &sector_info );
		if(ret_val == 0 || sector_info == NULL)
			return 1;

		/*find out how may sectors are found*/
		num = 0;
		size = 0;
		curr_sec = sector_info;
		while( curr_sec->size != 0 )
		{
			size += curr_sec->size;
			curr_sec++;
			num++;
		}
		m->memBufferType.Flash.numberOfSectors = num;
		m->memBufferType.Flash.algo = FLASH_ALGORITHEM_ST;
		m->memBufferType.Flash.chipsPerBus = ret_val;
		m->memBufferType.Flash.sectorList = sector_info;
		return 0;
	}
	else if( (m->busSize & 0xFF) == 132 )// Atmel
	{
		ret_val = jt_atmelflashGetInfo( m->baseAddr , &sector_info );
		if(ret_val == 0 || sector_info == NULL)
			return 1;
		
		/*find out how may sectors are found*/
		num = 0;
		size = 0;
		curr_sec = sector_info;
		while( curr_sec->size != 0 )
		{
			size += curr_sec->size;
			curr_sec++;
			num++;
		}
		m->memBufferType.Flash.numberOfSectors = num;
		m->memBufferType.Flash.algo = FLASH_ALGORITHEM_ATMEL;
		m->memBufferType.Flash.chipsPerBus = ret_val;
		m->memBufferType.Flash.sectorList = sector_info;
		return 0;
	}
	else if( (m->busSize & 0xFF) == 128 )// Philips
	{
		ret_val = jt_philipsflashGetInfo( m->baseAddr , m->length , &sector_info );
		if(ret_val == 0 || sector_info == NULL)
			return 1;
		
		/*find out how may sectors are found*/
		num = 0;
		size = 0;
		curr_sec = sector_info;
		while( curr_sec->size != 0 )
		{
			size += curr_sec->size;
			curr_sec++;
			num++;
		}
		m->memBufferType.Flash.numberOfSectors = num;
		m->memBufferType.Flash.algo = FLASH_ALGORITHEM_PHILIPS;
		m->memBufferType.Flash.chipsPerBus = ret_val;
		m->memBufferType.Flash.sectorList = sector_info;
		return 0;
	}

	return 1;
}

/*
 *
 *
 */
int activateMemMap(int num)
{
	int i, fd, size, ret, extra_space;
	struct memMapHead * mh;
	struct memMap *m;
	char tmpName[64];
	char *zeroBuf;
	char *mmptr;
	struct cmdSequence * cseq;
	struct cmdSequenceHead * sh;
	int active_workspace_mode;
	
#define PAGE_BUF 512*2

	mh = searchMemMapHead(num);
	if(mh == NULL || mh->numberOfEntrys <= 0)
		return EINVAL;

	zeroBuf = (char *) malloc(PAGE_BUF);
	if(zeroBuf == NULL)
		return ENOMEM;
	bzero(zeroBuf, PAGE_BUF);

	memMapContainer.workspace = NULL;
	/*run through list*/
	m = mh->firstMapEntry;
	for(i=0; 1 ; i++)
	{
		/*unmap old stuff*/
		if(m->memBuffer.Byte != NULL)
		{
			munmap(m->memBuffer.Word,m->memBufferLength);
			m->memBuffer.Byte = NULL;
			m->memBufferLength = 0;
		}
		/*MMAP - create a memory mapped file only for FLASH and RAM*/
		if(  m->type == MMAP_T_FLASH
		  || m->type == MMAP_T_APPLICATION_FLASH
		  || m->type == MMAP_T_UNREAL 
		  || m->type == MMAP_T_ROM 
		  || m->type == MMAP_T_RAM )
		{
			/* length must be multiple of RAM_PAGE_SIZE */
			if( (m->type == MMAP_T_ROM || m->type == MMAP_T_RAM )
			   && m->length != (m->length / RAM_PAGE_SIZE) * RAM_PAGE_SIZE)
			{
				m->length = (m->length / RAM_PAGE_SIZE) * RAM_PAGE_SIZE;
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"truncating length to %d\n",m->length);
				if(m->length <= 0)
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"length %d is too small\n",m->length);
					goto next_list_entry;
				}
			}
			snprintf(tmpName,sizeof(tmpName), ".mmap%d.tmp",i);
			fd = open(tmpName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
			if(fd < 0)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"can't create file %s\n",tmpName);
				exit(EX_CANTCREAT);
			}
			if( fchmod(fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) < 0 )
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"can't do a change mode on file %s\n",tmpName);
				exit(EX_CANTCREAT);
			}
			
			/*reserve extra space for ram write and read bitmaps*/
			if(m->type == MMAP_T_RAM)
			{
				/*Each bitstr_t element holds 8 bit data. So there it is one bit in the bitmap for each writen Byte*/
				/*The size of each page is  4 * 32 Byte = 128 Byte = 64 Halfwords = 32 Words*/
				extra_space = m->length/(8*sizeof(bitstr_t)) 
					    + m->length/(RAM_PAGE_SIZE*8*sizeof(bitstr_t)) + 2;
			}
			else if( m->type == MMAP_T_ROM)
				extra_space = m->length/(RAM_PAGE_SIZE*8*sizeof(bitstr_t)) + 1;
			else if(m->type == MMAP_T_FLASH || m->type == MMAP_T_APPLICATION_FLASH) 
				extra_space = m->length/(8*sizeof(bitstr_t)) + 1; //Flash has writebitmap only
			else // if( m->type == MMAP_T_UNREAL )
				extra_space = 0;
			
			/*avoid file fragmentation*/
			for(size = 0;(uint32_t)size < (m->length + extra_space);size += PAGE_BUF)
			{
				ret = write(fd,zeroBuf,PAGE_BUF);
				if(ret<0)
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"fail to write to file %s\n",tmpName);
					exit(EX_IOERR);
				}
			}
			mmptr = mmap(NULL, m->length + extra_space, PROT_READ | PROT_WRITE, /*MAP_PRIVATE*/ MAP_SHARED /*| MAP_NOSYNC*/, fd, 0);
			if(mmptr == MAP_FAILED)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"mmap failed\n");
				exit(EX_NOINPUT);
			}
			close(fd);
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Map Buffer to file %s\n",tmpName);
			m->memBuffer.Byte = mmptr;
			m->memBufferLength = m->length + extra_space;
		
			if(m->type == MMAP_T_FLASH || m->type == MMAP_T_APPLICATION_FLASH)
			{
				/*on every FLASH -> see which kind of Flash it is and allocate its sector info*/
				detectFlash(m);
				/*it migth be better to set the default memory to 0xFF*/
				memset(mmptr, 0xFF, m->length);
				m->memBufferType.Flash.writeByteBitmap = (bitstr_t *)mmptr + m->length;
				/*the bitmap is already zeroed so nothing more left*/
			}
			else if(m->type == MMAP_T_RAM) // it's MMAP_T_RAM
			{
				/*on every RAM -> allocate Buffer for bitmap*/
				m->memBufferType.Ram.readPageBitmap = (bitstr_t *)mmptr + m->length;
				m->memBufferType.Ram.writeByteBitmap = m->memBufferType.Ram.readPageBitmap + (m->length/(RAM_PAGE_SIZE*8*sizeof(bitstr_t))+1) * sizeof(bitstr_t);
				/*the bitmap's are already zeroed so nothing more left*/
			}
			else if(m->type == MMAP_T_ROM)// it's MMAP_T_ROM
			{
				m->memBufferType.Ram.readPageBitmap = (bitstr_t *)mmptr + m->length;
				m->memBufferType.Ram.writeByteBitmap = NULL;
			}
			else // it's MMAP_T_UNREAL
			{
				m->memBufferType.Ram.readPageBitmap = NULL;
				m->memBufferType.Ram.writeByteBitmap = NULL;
			}
		}
		else if(m->type == MMAP_T_WORKSPACE && memMapContainer.workspace == NULL)
		{	
			/*check if the work space big enouth to hold .text and stack*/
			if( is_workspace_big_enough (m->length))
			{
				memMapContainer.workspace = m;
				m->memBufferType.Workspace.state = WORKSPACE_UNTESTED;
				m->memBufferType.Workspace.info = STAND_ALONE_WORKSPACE;
			}
		}
next_list_entry:
		/*next List Entry*/
		if((i+1)>=mh->numberOfEntrys)
			break;
		m = m->nextMap;
	}
	memMapContainer.activeNumber	= num;
	memMapContainer.activeMap	= mh;
	free(zeroBuf);
	
	/*activate the new workspace*/
	active_workspace_mode = workspace_mode;
	workspace_mode = -1;
	changeWorkSpaceMode ( active_workspace_mode );
		
	/*make sure all old MemMaps in the cmdSequences are marked as invalid*/
	if(cmdSequenceContainer.numberOfCmdSequences <= 0)
		return 0; // nothing more to do

	/*all sequence Heads*/
	sh = cmdSequenceContainer.firstSequence;
	while(sh != NULL)
	{
		/*all Entrys*/
		cseq = sh->firstSequenceEntry;
	
		while(cseq != NULL)
		{
			cseq->memMapNumber = -1; // mark as no valid MemoryMap entry
			cseq->memMap	= NULL;
			cseq = cseq->nextSequence;
		}
		sh = sh->nextHead;
	}
	return 0;
}

/*
 *
 *
 */
void deactivateMemMap(void)
{
	int i;
	struct memMapHead * mh;
	struct memMap *m;
	struct cmdSequence * cseq;
	struct cmdSequenceHead * sh;
	
	mh = memMapContainer.activeMap;
	if(mh == NULL)
		return ;

	memMapContainer.workspace = NULL;
	/*run through list*/
	m = mh->firstMapEntry;
	for(i=0; 1 ; i++)
	{
		/*release flash sector list info*/
		if (m->type == MMAP_T_FLASH || m->type == MMAP_T_APPLICATION_FLASH)
		{
			if(m->memBufferType.Flash.sectorList != NULL)
				free(m->memBufferType.Flash.sectorList);
			m->memBufferType.Flash.sectorList = NULL;
			m->memBufferType.Flash.numberOfSectors = 0;
			m->memBufferType.Flash.chipsPerBus = 0;
			m->memBufferType.Flash.algo = FLASH_ALGORITHEM_UNKOWN;
		}
		/*unmap old stuff*/
		if(m->memBuffer.Byte != NULL)
		{
			munmap(m->memBuffer.Word,m->memBufferLength);
			m->memBuffer.Byte = NULL;
			m->memBufferLength = 0;
		}
		/*next List Entry*/
		if((i+1)>=mh->numberOfEntrys)
			break;
		m = m->nextMap;
	}
	memMapContainer.activeNumber = -1;
	memMapContainer.activeMap = NULL;

	/*all sequence Heads*/
	sh = cmdSequenceContainer.firstSequence;
	while(sh != NULL)
	{
		/*all Entrys*/
		cseq = sh->firstSequenceEntry;
	
		while(cseq != NULL)
		{
			cseq->memMapNumber = -1; // mark as no valid MemoryMap entry
			cseq->memMap	= NULL;
			cseq = cseq->nextSequence;
		}
		sh = sh->nextHead;
	}

	return;
}



/*
 * Callback function to print memory map info to gdb
 */
int printmemMapCB_info(int currMemMapHeadCnt, int currEntryCnt, struct context *contextSrc)
{
	struct memMapCBContext *context = (struct memMapCBContext *)contextSrc;
	struct memMap * mm;
	struct cmdCallbackEntry *cbEntry;
	struct sector	*secList;
	char *infoString;
	int idx;
	struct gdbSprintfBuf *msg_buf;

	if(context == NULL)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"missing context\n");
		return -1;
	}
	
	/*make sure we are still in debug state since we need the traget CPU*/
	if ((ice_state.is_debugrequest & 1) == 1)
	{
		/*do we are going to send messages to the gdb console*/
		if(fake_continue_mode)
			msg_buf = allocateGdbSprintfBuf(BUFMAX);
		else
			msg_buf = NULL;

		if (currEntryCnt > 0) /*check if there is data inside of the Map*/
		{
			if(currEntryCnt == 1 && (context->curr_secCnt == 0 || context->curr_secCnt == 1))
				gdbPrintf(ACTION_NON, 0, msg_buf, "MemMap[%d]:\n",currMemMapHeadCnt-1);
			mm = context->curr_memMap;
			switch (mm->type)
			{
			case MMAP_T_UNUSED:
				break;
			case MMAP_T_FLASH:
			case MMAP_T_APPLICATION_FLASH:
				if(context->curr_secCnt == 0)
				{
					gdbPrintf(ACTION_NON
						, 0
						, msg_buf
						, "[%d] @0x%X\t(%d) Bytes <%d>\tinactive Flash type\n"
						, currEntryCnt - 1
						, mm->baseAddr
						, mm->length
						, mm->busSize
						);
				}
				else
				{
					secList = mm->memBufferType.Flash.sectorList;
					if(secList == NULL) //should not happen -- paranoia test
					{
						dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"missing sector list");
						return -1;
					}

					if(context->curr_secCnt == 1)
					{
						switch (mm->memBufferType.Flash.algo)
						{
						case FLASH_ALGORITHEM_AMD:	
							infoString = "AMD";
							break;
						case FLASH_ALGORITHEM_INTEL:
							infoString = "Intel";
							break;
						case FLASH_ALGORITHEM_PHILIPS:
							infoString = "Philips";
							break;
						case FLASH_ALGORITHEM_ATMEL:
							infoString = "Atmel";
							break;
						case FLASH_ALGORITHEM_ST:
							infoString = "STM";
							break;
						case FLASH_ALGORITHEM_UNKOWN:
						case FLASH_ALGORITHEM_NOSUPPORT:
						default:
							infoString = "unknown";
							break;
						}
						gdbPrintf(ACTION_NON
							, 0
							, msg_buf
							, "[%d] @0x%X\t(%d) Bytes <%d>\t%s Flash with #%d segments\n"
							, currEntryCnt - 1
							, mm->baseAddr
							, mm->length
							, mm->busSize
							, infoString
							, mm->memBufferType.Flash.numberOfSectors
							);
					}
					idx = context->curr_secCnt - 1;
					switch(secList[idx].valid)
					{
					case SECTOR_INVALID:
						infoString = "unchecked";
						break;
					case SECTOR_READ_VALID:		
						/* read from target- so it is valid */
						infoString = "valid";
						break;
					case SECTOR_READ_ERASED_VALID:	
						/* read from target but all data are erased*/
						infoString = "valid (Flash erased)";
						break;
					case SECTOR_WRITE:		
						/* nither yet written, nor checked */
						infoString = "modified unchecked";
						break;
					case SECTOR_WRITE_DURTY:	
						/* not yet written, but known to be dury */
						infoString = "modified durty";
						break;
					case SECTOR_WRITE_DURTY_ERASED:	
						/* not yet written, but known to be dury and Flash is already erased*/
						infoString = "modified durty (Flash erased)";
						break;
					case SECTOR_FLAG_RW:
					case SECTOR_FLAG_DURTY:
					case SECTOR_FLAG_ERASED:
					default:
						/*other flag combinations should not happen*/
						infoString = "unknown state";
						break;
					}
					gdbPrintf(ACTION_NON
						, 0
						, msg_buf
						, "\tseg %d:\tsize: %d\t<%s>\n"
						, idx
						, secList[idx].size
						, infoString
						);
				}
				break;
			case MMAP_T_ROM:
				gdbPrintf(ACTION_NON
					, 0
					, msg_buf
					, "[%d] @0x%X\t(%d) Bytes <%d>\tROM\n"
					, currEntryCnt - 1
					, mm->baseAddr
					, mm->length
					, mm->busSize
					);
				break;
			case MMAP_T_RAM:
				gdbPrintf(ACTION_NON
					, 0
					, msg_buf
					, "[%d] @0x%X\t(%d) Bytes <%d>\tRAM\n"
					, currEntryCnt - 1
					, mm->baseAddr
					, mm->length
					, mm->busSize
					);
				break;
			case MMAP_T_UNREAL:
				gdbPrintf(ACTION_NON
					, 0
					, msg_buf
					, "[%d] @0x%X\t(%d) Bytes <%d>\tUnreal (to make GDB happy)\n"
					, currEntryCnt - 1
					, mm->baseAddr
					, mm->length
					, mm->busSize
					);
				break;
			case MMAP_T_SFA:
				gdbPrintf(ACTION_NON
					, 0
					, msg_buf
					, "[%d] @0x%X\t(%d) Bytes <%d>\tSpecial Function Area\n"
					, currEntryCnt - 1
					, mm->baseAddr
					, mm->length
					, mm->busSize
					);
				break;
			case MMAP_T_IO:
				gdbPrintf(ACTION_NON
					, 0
					, msg_buf
					, "[%d] @0x%X\t(%d) Bytes <%d>\tIO\n"
					, currEntryCnt - 1
					, mm->baseAddr
					, mm->length
					, mm->busSize
					);
				break;
			case MMAP_T_WORKSPACE:
				gdbPrintf(ACTION_NON
					, 0
					, msg_buf
					, "[%d] @0x%X\t(%d) Bytes <%d>\tWorkspace\n"
					, currEntryCnt - 1
					, mm->baseAddr
					, mm->length
					, mm->busSize
					);
				break;
			case MMAP_T_CACHE:
				gdbPrintf(ACTION_NON
					, 0
					, msg_buf
					, "[%d] @0x%X\t(%d) Bytes <%d>\tCache\n"
					, currEntryCnt - 1
					, mm->baseAddr
					, mm->length
					, mm->busSize
					);
				break;
			default:
				gdbPrintf(ACTION_NON, 0, msg_buf, "unknown type\n");
			}
			
		}

		/*setup data for next info to print*/
		if (context->curr_secCnt < context->max_secCnt) /*next flash sector*/
		{
			context->curr_secCnt++;
		}
		else if(currEntryCnt < context->max_MapEntry_number) /*next memMap entry*/
		{
			currEntryCnt++;
			context->curr_memMap = context->curr_memMap->nextMap;
			mm = context->curr_memMap;
			goto common_setup_sector;
		}
		else if (currMemMapHeadCnt < context->max_MemMap_number) /*next Map*/
		{
			currMemMapHeadCnt++;
			context->curr_Map = context->curr_Map->nextHead;
			context->curr_memMap = context->curr_Map->firstMapEntry;
			context->max_MapEntry_number = context->curr_Map->numberOfEntrys;
			if (context->max_MapEntry_number > 0)
			{
				currEntryCnt = 1;  // start with first entry
				mm = context->curr_memMap;
common_setup_sector:		if (mm->type == MMAP_T_FLASH || mm->type == MMAP_T_APPLICATION_FLASH)
				{
					context->max_secCnt = mm->memBufferType.Flash.numberOfSectors;
					if (context->max_secCnt > 0)
						context->curr_secCnt = 1;  // start with first sector
					else
						context->curr_secCnt = 0;  // seems to be broken
				}
				else
				{
					context->max_secCnt = 0;
					context->curr_secCnt = 0;
				}
			}
			else
			{
				currEntryCnt = 0;  // skip empty Map			
				context->max_secCnt = 0;
				context->curr_secCnt = 0;
			}
		}
		else
		{
			/*last*/
			gettimeofday(&(memMapContainer.memStat.stopTime), NULL);
			memMapContainer.memStat.pending = ACTION_MEMMAPINFO_SUCCEED;
			if(msg_buf)
					inQueueGdbSprintfBuf(msg_buf);
			return 0;
		}
		if(msg_buf)
			inQueueGdbSprintfBuf(msg_buf);
	}
	else
	{
		memMapContainer.memStat.pending = ACTION_MEMMAPINFO_FAIL;
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"must be in debug state\n");
		return 1;
	}
		
	/*create context of next callback*/
	cbEntry = allocateCmdCallbackEntry(sizeof(struct memMapCBContext));
	if(cbEntry == NULL)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"no Mem for new Entry\n");
		return -1;
	}

	inQueueCmdCallbackEntry(
				cbEntry,
				printmemMapCB_info,
				currMemMapHeadCnt,
				currEntryCnt,
				contextSrc,
				sizeof(struct memMapCBContext)
				);
	return 0;
}

