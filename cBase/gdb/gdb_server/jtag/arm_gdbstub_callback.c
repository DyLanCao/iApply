/*
 * arm_gdbstub_callback.c
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>

#include <unistd.h>
//#include <signal.h>
//#include <sys/wait.h>

//#include <ctype.h>
//#include <sysexits.h>

#include "dbg_msg.h"
//#include "jt_instr.h" 
#include "jt_arm.h"
#include "jt_flash.h"
#include "arm_gdbstub.h"

struct cmdCallbackConainer cmdCallbackConainer;

/*
 * callback stuff (- instead of multi threading)
 */
struct cmdCallbackEntry *allocateCmdCallbackEntry(int byteSizeOfExtraContext)
{
	uint8_t *context;
	struct cmdCallbackEntry *entry;

	if(byteSizeOfExtraContext > 0)
	{
		context = (uint8_t *) malloc(byteSizeOfExtraContext);
		if(context == NULL)
			return NULL;
	}
	else
		context = NULL;

	entry = (struct cmdCallbackEntry *) malloc(sizeof(struct cmdCallbackEntry));
	if(entry == NULL)
	{
		if(context)
			free(context);
		return NULL;
	}

	if(context)
		bzero(context, byteSizeOfExtraContext);
	bzero(entry, sizeof(struct cmdCallbackEntry));

	entry->context = (struct context *) context;
	return entry;
}

void freeCmdCallbackEntry(struct cmdCallbackEntry *cb)
{
	if(cb == NULL)
		return;
	if(cb->context != NULL)
		free(cb->context);
	free(cb);
	return;
}

void inQueueCmdCallbackEntry(
		struct cmdCallbackEntry *entry,
		int (*fntCB) (int arg, int cnt,struct context *context),
		int argSrc,
		int cntSrc,
		struct context *contextSrc,
		int contextSize
		)
{
	struct cmdCallbackEntry *cur;
	
	if(entry == NULL)
		return;

	if(cmdCallbackConainer.numberOfCBentrys <= 0)
	{
		cmdCallbackConainer.firstCBentry = entry;
		cmdCallbackConainer.lastCBentry = entry;
		cmdCallbackConainer.numberOfCBentrys = 1;
	}
	else
	{
		cur = cmdCallbackConainer.lastCBentry;
		entry->prevCBentry = cur;
		cur->nextCBentry = entry;
		entry->nextCBentry = NULL; //paranoia setting
		cmdCallbackConainer.lastCBentry = entry;
		cmdCallbackConainer.numberOfCBentrys++;
	}
	
	/*also insert info of parmeter's*/
	if(fntCB)
		entry->fntCB = fntCB;

	if(argSrc)
		entry->arg = argSrc;

	if(cntSrc)
		entry->cnt = cntSrc;

	if( contextSrc != NULL && contextSize > 0 )
	{
		if(entry->context == NULL)
		{	
			entry->context = (struct context *) malloc(contextSize);
			if(entry->context == NULL)
				return;
		}
		bcopy(contextSrc, entry->context, contextSize);
	}

	return;		
}


struct cmdCallbackEntry * deQueueCmdCallbackEntry(void)
{
	struct cmdCallbackEntry *cur;
	
	if(cmdCallbackConainer.numberOfCBentrys <= 0)
		return NULL;

	cur = cmdCallbackConainer.firstCBentry;

	cmdCallbackConainer.firstCBentry = cur->nextCBentry;
	cur->nextCBentry = NULL;
	if(cmdCallbackConainer.firstCBentry != NULL)
		cmdCallbackConainer.firstCBentry->prevCBentry = NULL;
	cmdCallbackConainer.numberOfCBentrys--;
	if(cmdCallbackConainer.numberOfCBentrys <= 0)
	{
		cmdCallbackConainer.numberOfCBentrys = 0;
		cmdCallbackConainer.lastCBentry = NULL;
	}
	return cur;
}

int doCallback(void)
{
	struct cmdCallbackEntry *entry;
	int ret_val;

	entry = deQueueCmdCallbackEntry();
	if(entry == NULL)
		return 0; // Nothing to do; so it is OK
	if(entry->fntCB == NULL)
	{
		freeCmdCallbackEntry(entry);
		return -1; // no function to call
	}
	ret_val = entry->fntCB(entry->arg, entry->cnt, entry->context);
	freeCmdCallbackEntry(entry);
	return ret_val;
}

void removeAllCallbacks(void)
{
	struct cmdCallbackEntry *entry;

	while((entry = deQueueCmdCallbackEntry()) != NULL)
	{
		freeCmdCallbackEntry(entry);
	}
	return;
}

/*
 * send Stringbuffer to gdb's stderr while doing a fake conituing operation
 */

struct gdbSprintfBufContainer gdbSprintfBufContainer = {NULL,NULL};

/*
 *
 */
struct gdbSprintfBuf *allocateGdbSprintfBuf(int size_of_string)
{
	struct gdbSprintfBuf *entry;
	char *str;

	/*deny useless stuff*/
	if(size_of_string <= 0)
		return NULL;
	
	/*allocate space for internal data*/
	entry = (struct gdbSprintfBuf *) malloc(sizeof(struct gdbSprintfBuf));

	if(entry == NULL)
		return NULL;

	/*allocate space for string data*/
	if(size_of_string < BUFMAX/2)
		size_of_string = BUFMAX/2;

	str = (char *) malloc(size_of_string+1);

	if(str == NULL)
	{
		free(entry);
		return NULL;
	}

	/*fill in default data*/
	str[0] = 0;
	entry->len = 0;
	entry->next = NULL;
	entry->string = str;
	return entry;
}

	
void freeGdbSprintfBuf(struct gdbSprintfBuf *buf)
{
	if(buf == NULL)
		return;
	
	if(buf->string != NULL)
		free(buf->string);
	free(buf);
	return;
}

/*
 * place buffer entry at the tail of the queue
 */
void inQueueGdbSprintfBuf(struct gdbSprintfBuf *buf)
{
	if(buf == NULL) // nothing to do
		return;

	if(gdbSprintfBufContainer.first == NULL)
		gdbSprintfBufContainer.first = buf;
	else
		gdbSprintfBufContainer.last->next = buf;	
	gdbSprintfBufContainer.last  = buf;
	buf->next = NULL; //paranoia setting
	return;
}

/*
 * collect head of queue and remove head from queue
 * return head
 */
struct gdbSprintfBuf *deQueueGdbSprintfBuf(void)
{
	struct gdbSprintfBuf *head;

	head = gdbSprintfBufContainer.first;
	if(head != NULL)
	{
		if(gdbSprintfBufContainer.first == gdbSprintfBufContainer.last)
		{
			gdbSprintfBufContainer.first = NULL;
			gdbSprintfBufContainer.last  = NULL;
		}
		else
		{
			gdbSprintfBufContainer.first = head->next;
		}
		head->next = NULL;
	}
	return head;
}

/*
 * pack string buffer together
 */
void packGdbSprintBuf(void)
{
	struct gdbSprintfBuf *curr_search, *prev_search, *curr_pack;

	prev_search = NULL;
	curr_search = curr_pack = gdbSprintfBufContainer.first;
	while(curr_search && curr_pack)
	{
		/*are we able to tranfer the string data from curr_search into curr_pack*/
		if(curr_search != curr_pack) 
		{
			if((BUFMAX/2 - curr_pack->len) > curr_search->len )
			{
				/*we have enough space in Pack -- now transfer curr into it*/
				if(curr_search->len > 0)
				{
					memcpy(  &curr_pack->string[curr_pack->len]
						,&curr_search->string[0] 
						,curr_search->len );
					curr_pack->len += curr_search->len;
				}
				curr_search->len = 0;
				
				if(prev_search)
				{
					/*erase now empty curr out of the list*/
					prev_search->next = curr_search->next;
					freeGdbSprintfBuf(curr_search);
					curr_search = prev_search->next;
					if(curr_search == NULL) // the erased one was the last
						gdbSprintfBufContainer.last  = prev_search;
				}
				else // this can never happen
				{
					/*store prev search*/
					prev_search = curr_search;
					/*next search*/
					curr_search = curr_search->next;
				}
			}
			else
			{
				/*make sure curr is at least one ahead to pack*/
				if(curr_search == curr_pack->next)
				{
					/*store prev search (to be able to delete the next if it gets empty)*/
					prev_search = curr_search;
					/*next search*/
					curr_search = curr_search->next;
				}
				
				/*next pack*/
				curr_pack = curr_pack->next;
			}
		}
		else
		{
			/*store prev search (to be able to delete the next if it gets empty)*/
			prev_search = curr_search;
			/*next search*/
			curr_search = curr_search->next;
		}
	}
	return;
}

/*
 * Printf wrapper
 */
void gdbPrintf(int set_pending_action, int doInQueue, struct gdbSprintfBuf *msg_buf, const char *format, ...)
{
	va_list argListp;

	va_start(argListp, format);
	IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
		vprintf(format, argListp);
	
	if((set_pending_action & ACTION_TERMINATE) == ACTION_TERMINATE )
	{
		gettimeofday(&(memMapContainer.memStat.stopTime), NULL);
		memMapContainer.memStat.pending = set_pending_action;
	}
	else if (set_pending_action != ACTION_NON)
		memMapContainer.memStat.pending = set_pending_action;

	if(msg_buf)
	{
		if(BUFMAX/2 > msg_buf->len + 1)
		{
			vsnprintf(&msg_buf->string[msg_buf->len]
				,BUFMAX/2 - 1 - msg_buf->len
				,format , argListp);
			msg_buf->len += strlen(&msg_buf->string[msg_buf->len]);
			if(msg_buf->len > BUFMAX/2)
				msg_buf->len = BUFMAX/2 - 1;
		}
		if (doInQueue)
			inQueueGdbSprintfBuf(msg_buf);
	}
	va_end(argListp);
	return;
}

