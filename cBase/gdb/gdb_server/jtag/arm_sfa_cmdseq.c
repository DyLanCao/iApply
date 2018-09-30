/*
 * arm_sfa_cmdseq.c
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
struct cmdSequenceContainer cmdSequenceContainer = {0, NULL};

int cmdSequenceNumber_StartPeriphery = -1;
int cmdSequenceNumber_StopPeriphery  = -1;
int cmdSequenceNumber_ResetCPU  = -1;

int AllocCmdSeqences(int total_number_of_cmdSequences)
{
	int add_nmbr, i;
	struct cmdSequenceHead *sh, *curr_sh, *prev_sh;

	if(cmdSequenceContainer.numberOfCmdSequences >= total_number_of_cmdSequences)
		return 0; // nothing to do
	
	add_nmbr = total_number_of_cmdSequences - cmdSequenceContainer.numberOfCmdSequences;
	sh = (struct cmdSequenceHead *) malloc(add_nmbr * sizeof(struct cmdSequenceHead));
	if(sh == NULL)
		return ENOMEM;
	bzero(sh,add_nmbr * sizeof(struct cmdSequenceHead));

	/*find last cmdSequenceHead*/
	curr_sh = cmdSequenceContainer.firstSequence;
	prev_sh = NULL;
	while(curr_sh != NULL)
	{
		prev_sh = curr_sh;
		curr_sh = curr_sh->nextHead;
	}

	if(prev_sh == NULL) // it was the first in the container
		cmdSequenceContainer.firstSequence = sh;
	else
		prev_sh->nextHead = sh;

	/*generete missing list*/
	for(i=1; i<add_nmbr; i++)
	{
		sh->nextHead = &sh[1];
		sh++;
	}
	cmdSequenceContainer.numberOfCmdSequences = total_number_of_cmdSequences;
	return 0;
}

	
struct cmdSequenceHead * searchCmdSequenceHead(int num)
{
	int i;
	struct cmdSequenceHead *sh;

	if(num < 0 || num >= cmdSequenceContainer.numberOfCmdSequences)
		return NULL;
	
	sh = cmdSequenceContainer.firstSequence;
	if(num == 0)
		return sh;
	
	for(i=0; i<num; i++)
		sh = sh->nextHead;
	return sh;
}

int AllocCmdSequenceEntrys(struct cmdSequenceHead * sh, int total_num)
{
	int add_nmbr, i;
	struct cmdSequence *s, *curr_s, *prev_s;

	if(sh == NULL)
		return EINVAL;
	if(sh->numberOfEntrys >= total_num)
		return 0;

	add_nmbr = total_num - sh->numberOfEntrys;
	s = (struct cmdSequence *) malloc(add_nmbr * sizeof(struct cmdSequence));
	if(s == NULL)
		return ENOMEM;
	bzero(s,add_nmbr * sizeof(struct cmdSequence));
	s->memMapNumber = -1; // mark: no valid MemoryMap entry
	
	/*find last cmdSequence*/
	curr_s = sh->firstSequenceEntry;
	prev_s = NULL;
	while(curr_s != NULL)
	{
		prev_s = curr_s;
		curr_s = curr_s->nextSequence;
	}

	if(prev_s == NULL) // it was the first in the Head
		sh->firstSequenceEntry = s;
	else
		prev_s->nextSequence = s;

	/*generate missing list*/
	for(i=1; i<add_nmbr; i++)
	{
		s->nextSequence = &s[1];
		s->memMapNumber = -1; // mark: no valid MemoryMap entry
		s++;
	}
	sh->numberOfEntrys = total_num;
	return 0;
	
}

struct cmdSequence * searchCmdSequenceEntry(struct cmdSequenceHead * sh, int num)
{
	int i;
	struct cmdSequence *s;

	if(sh == NULL)
		return NULL;
	
	if(num >= sh->numberOfEntrys || num < 0)
		return NULL;
	
	s = sh->firstSequenceEntry;
	if(num == 0)
		return s;
	
	for(i=0; i<num; i++)
		s = s->nextSequence;
	return s;
}
	
int updateCmdSequence(struct cmdSequenceHead * sh, int num, uint32_t cmdSize, uint32_t addr, uint32_t val, enum CmdSequenceFlag flag)
{
	struct cmdSequence * cseq;

	cseq = searchCmdSequenceEntry(sh ,num);
	if(cseq == NULL)
		return EINVAL;
		
	cseq->cmdSize	= cmdSize;
	cseq->addr	= addr;
	cseq->val	= val;
	cseq->flag	= flag;
	cseq->memMapNumber = -1; // mark as no valid MemoryMap entry
	cseq->memMap	= NULL;
	return 0;
}

uint32_t readvalCmdSequence(int num, int entryNum)
{
	struct cmdSequence * cseq;
	struct cmdSequenceHead * sh;

	/*we need an activated MemMap*/
	if(memMapContainer.activeNumber < 0)
		return 0;
	
	sh = searchCmdSequenceHead(num);
	if(sh == NULL || sh->numberOfEntrys <= 0)
		return 0;
	
	cseq = searchCmdSequenceEntry(sh ,entryNum);
	if(cseq == NULL)
		return 0;
		
	return cseq->val;
}

uint32_t readaddrCmdSequence(int num, int entryNum)
{
	struct cmdSequence * cseq;
	struct cmdSequenceHead * sh;

	/*we need an activated MemMap*/
	if(memMapContainer.activeNumber < 0)
		return 0;
	
	sh = searchCmdSequenceHead(num);
	if(sh == NULL || sh->numberOfEntrys <= 0)
		return 0;
	
	cseq = searchCmdSequenceEntry(sh ,entryNum);
	if(cseq == NULL)
		return 0;
		
	return cseq->addr;
}

int doCmdSequence(int num)
{
	struct cmdSequence * cseq;
	struct cmdSequenceHead * sh;
	struct memMap * m;
	int i;
	
	/*we need an activated MemMap*/
	if(memMapContainer.activeNumber < 0)
		return ENOTSUP;
	
	sh = searchCmdSequenceHead(num);
	if(sh == NULL || sh->numberOfEntrys <= 0)
		return EINVAL;

	/*run through sequence*/
	cseq = sh->firstSequenceEntry;
	for(i=0; i<sh->numberOfEntrys; i++)
	{
		/*lookup Memmory Map*/
		if(cseq->memMapNumber != memMapContainer.activeNumber || cseq->memMap == NULL)
		{
			m = findMemMapOfAddr(cseq->addr);
			if(m == NULL)
			{
				cseq = cseq->nextSequence;
				continue;
			}
			cseq->memMapNumber = memMapContainer.activeNumber;
			cseq->memMap = m;
		}
		else
			m = cseq->memMap;

		/*check if in SFA*/
		if(m->type == MMAP_T_SFA)
		{
			if(cseq->flag == CMDSEQUFLAG_WRITE)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"(%d) (@ 0x%8.8X) -> 0x%8.8X\n",cseq->cmdSize, cseq->addr, cseq->val);

				if(cseq->cmdSize == 32)
					jtag_arm_WriteWord(cseq->addr, cseq->val);
				else if(cseq->cmdSize == 16)
					jtag_arm_WriteHalfword(cseq->addr, cseq->val);
				else if(cseq->cmdSize == 8)
					jtag_arm_WriteByte(cseq->addr, cseq->val);
				else
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"wrong Bus bit size %d for command 0x%8.8X (@ 0x%8.8X)\n",cseq->cmdSize, cseq->val, cseq->addr);
			}
			else // CMDSEQUFLAG_READ
			{
				if(cseq->cmdSize == 32)
					cseq->val = jtag_arm_ReadWord(cseq->addr);
				else if(cseq->cmdSize == 16)
					cseq->val = jtag_arm_ReadHalfword(cseq->addr);
				else if(cseq->cmdSize == 8)
					cseq->val = jtag_arm_ReadByte(cseq->addr);
				else
					dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"wrong Bus bit size %d (@ 0x%8.8X)\n",cseq->cmdSize, cseq->addr);
					
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"(%d) (@ 0x%8.8X) <- 0x%8.8X\n",cseq->cmdSize, cseq->addr, cseq->val);
			}
		}
		
		cseq = cseq->nextSequence;
	}
	
	return 0;
}

/*
 *   Start Periphery after leave gdb prompt (stepping or continuing) 
 * 
 */
void arm_sfa_StartPeriphery(void) 
{
	doCmdSequence(cmdSequenceNumber_StartPeriphery);
	return;
}

/*
 *   Stop Periphery before enter to gdb prompt 
 * 
 */
void arm_sfa_StopPeriphery(void)
{
	doCmdSequence(cmdSequenceNumber_StopPeriphery);
	return;
}

/*
 *   Sequence to Reset CPU
 */
void arm_sfa_ResetCPU(void)
{
	doCmdSequence(cmdSequenceNumber_ResetCPU);
	return;
}

